#include "db_mysql.h"
#include "easylogging++.h"
#include <utility>

DBMysql::DBMysql()
{
	connect_ = nullptr;
}

DBMysql::~DBMysql()
{
	delete connect_;
}


bool DBMysql::OpenDB()
{
	if (!connect_)
		return false;

	if (mysql_init(&mysql_) == NULL) 
    {
		return false;
	}

	if (!mysql_real_connect(&mysql_, connect_->url.c_str(), connect_->user_name.c_str(),
							connect_->user_pass.c_str(),connect_->use_db.c_str(), 
							connect_->port, NULL, 0))
	{
		std::string error= mysql_error(&mysql_);
		LOG(ERROR) << "openDB : " << "数据库连接失败:"<<error;
		return false;
	}

	return true;
}

void DBMysql::SetConnect(MysqlConnect*connect)
{
	connect_ = connect;
}

void DBMysql::GetConnect(MysqlConnect*connect)
{
	connect = connect_;
}

bool DBMysql::DoSqlQuery(const std::string& sql_query)
{
    int ret = mysql_real_query(&mysql_, sql_query.c_str(), strlen(sql_query.c_str()));
    if (ret != 0 && mysql_errno(&mysql_) != 1062) 
    {
        LOG(INFO) << "exec sql failed" << sql_query ;
        return false;
    }   
    return true;
}

void DBMysql::InsertData(const std::string& insert_sql)
{
	int ret = mysql_real_query(&mysql_, insert_sql.c_str(), strlen(insert_sql.c_str()));
	if (ret != 0 && mysql_errno(&mysql_) != 1062) 
	{
		LOG(INFO) << "exec sql failed" << insert_sql ;
	}	

}

bool DBMysql::GetDataAsJson(const std::string& select_sql, JsonDataFormat* json_data_format ,json& json_data)
{
    if (json_data_format->column_size != json_data_format->map_column_type.size())
    {
        LOG(ERROR) << "json data format size error!";
        return false;
    }


    int ret = mysql_real_query(&mysql_,select_sql.c_str(), strlen(select_sql.c_str()));
    MYSQL_RES *result = mysql_store_result(&mysql_);
    size_t num_rows = mysql_num_rows(result);

    for (size_t i = 0; i < num_rows; ++i) 
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        json row_data = json::array();

        for (size_t j = 0; j < json_data_format->column_size; j++)
        {
            std::string sql_data = row[j];
            JsonDataType type  = json_data_format->map_column_type[j];
            if ( INT == type )
            {
                int real_data = std::stoi(sql_data);
                row_data.push_back(real_data);
            }
            else if ( DOUBLE == type )
            {
                double real_data = std::stof(sql_data);
                row_data.push_back(real_data);
            }
            else if ( STRING == type )
            {
                row_data.push_back(sql_data);
            }
        }
    }
    mysql_free_result(result);

    return true;
}

bool DBMysql::FormatSql(const std::string& table_name, const std::string& sql_content, SqlFormat sql_format , std::string& sql_out)
{
    sql_out.empty();
    switch (sql_format)
    {
    case SQL_SELECT:
        sql_out = "select " + sql_content + " from " +table_name;
        break;
    case SQL_INSERT:
        sql_out = "insert into " + table_name + sql_content;
        break;
    case SQL_UPDATE:
        sql_out = "update " + table_name + " set " + sql_content;
        break;
    case SQL_DELETE:
        sql_out = "delete from " + table_name +  " where " + sql_content;
        break;
    default:
        break;
    }

    return true;
}

void DBMysql::AppendSqlCondition(const std::string& sql_content, std::string& sql_out, bool have_where /*= false*/)
{
    if (have_where)
    {
        sql_out = sql_out + " " + sql_content;
    }
    else 
    {
        sql_out = sql_out + " where " + sql_content;
    }
}

uint64_t DBMysql::GetMaxOffset(const std::string&sql_select)
{
	int ret = mysql_real_query(&mysql_,sql_select.c_str(), strlen(sql_select.c_str()));
	MYSQL_RES *result = mysql_store_result(&mysql_);
	size_t num_rows = mysql_num_rows(result);

	int offset = 0;
	for (size_t i = 0; i < num_rows; ++i) 
	{
		MYSQL_ROW row = mysql_fetch_row(result);
		std::string str_offset = row[0];
		if ( str_offset == "")
		{
			continue;
		}
		offset = std::atoi(str_offset.c_str());	
    }
	mysql_free_result(result);
	return offset;
}


void DBMysql::GetExchangeBalance(const uint64_t& interval_time,const uint32_t& sql_mod,std::vector<std::string>& vect_sql_insert)
{

	std::string sql_prefix = "select a.user_id,a.balance from balance_history_";

	std::string sql_prefix_in = " a join ( SELECT max(id) max_id, user_id FROM balance_history_";

	std::string sql_sniffix = " WHERE  time < " + std::to_string(interval_time) + " and id in ( select id from balance_history_";

	std::string sql_sniffix_in = " where asset = 'BAC' AND user_id  in (select DISTINCT user_id from balance_history_";

	std::string sql_tail = " where asset = 'BAC') ) group by user_id order by user_id ASC ) b on  a.id = b.max_id;";
	std::string sql_insert_mutil = "INSERT INTO `balance_snapshot`  ( `time`, `user_id`, `balance`) VALUES ";
	for(uint32_t i = 0; i < sql_mod; i ++)
	{
		std::string sql_flag = std::to_string(i);
		std::string sql_select = sql_prefix + sql_flag + sql_prefix_in + sql_flag + sql_sniffix + sql_flag + sql_sniffix_in + sql_flag + sql_tail;
		int ret = mysql_real_query(&mysql_,sql_select.c_str(), strlen(sql_select.c_str()));
		MYSQL_RES *result = mysql_store_result(&mysql_);

		size_t num_rows = mysql_num_rows(result);
		for (size_t j = 0; j < num_rows; ++j) 
		{
			MYSQL_ROW row = mysql_fetch_row(result);
			std::string user_id = row[0];
			std::string balance = row[1];
			std::string sql_insert ;
			if ( i == sql_mod - 1 && j == num_rows - 1 )
			{
				sql_insert =/*"INSERT INTO `balance_snapshot`  ( `time`, `user_id`, `balance`) VALUES ("*/ "(" + std::to_string(interval_time) + "," +user_id +","+balance +");";
			}
			else 
			{
				sql_insert =/*"INSERT INTO `balance_snapshot`  ( `time`, `user_id`, `balance`) VALUES ("*/ "(" + std::to_string(interval_time) + "," +user_id +","+balance +"),";
			}
			sql_insert_mutil = sql_insert_mutil + sql_insert;
			//vect_sql_insert.push_back(sql_insert);
		}
		mysql_free_result(result);
	}
	vect_sql_insert.push_back(sql_insert_mutil);

}

void DBMysql::SetBacServerBalance(const std::vector<std::string>& vect_sql_insert)
{
//	mysql_query(&mysql_,"START TRANSACTION");
	for(uint32_t i = 0; i < vect_sql_insert.size(); i ++)
	{
		InsertData(vect_sql_insert.at(i));
	}
//	mysql_query(&mysql_,"COMMIT");
}

void DBMysql::CloseDB()
{
	mysql_close(&mysql_);
}
