#ifndef DB_MYSQL_H
#define DB_MYSQL_H
#include <string>
#include <mysql/mysql.h>
#include <vector>
#include <map>
#include "json.hpp"
using json = nlohmann::json;
class DBMysql
{
public:
	DBMysql();
	~DBMysql();

	struct MysqlConnect
	{
		std::string url;
		int			port;
		std::string user_name;
		std::string user_pass;
		std::string use_db;
	};

    enum JsonDataType
    {
        INT = 1,
        DOUBLE = 2,
        STRING =3
    };
    
    struct JsonDataFormat
    {
        uint32_t column_size;
        std::map<uint32_t, JsonDataType> map_column_type;

    };

    enum SqlFormat
    {
        SQL_SELECT = 1,
        SQL_INSERT = 2,
        SQL_UPDATE = 3,
        SQL_DELETE = 4,
        SQL_DROP = 5
    };

public:
    static bool FormatSql(const std::string& table_name, const std::string& sql_content, SqlFormat sql_format , std::string& sql_out);

    static void AppendSqlCondition(const std::string& sql_content, std::string& sql_cout, bool have_where = false);

public:
    bool GetDataAsJson(const std::string& select_sql, JsonDataFormat* json_data_format ,json& json_data);

public:
    bool DoSqlQuery(const std::string& sql_query); 

public:	
 	bool OpenDB();

	void CloseDB();

	void SetConnect(MysqlConnect*connect);

	void GetConnect(MysqlConnect*connect);

public:

	void InsertData(const std::string&insert_sql);

	uint64_t GetMaxOffset(const std::string&sql_select);

	void GetFeeRateFrom(const std::string& stock, const std::string& money,const uint64_t& time,std::string& ask_fee_rate,std::string& bid_fee_rate);

	void GetExchangeBalance(const uint64_t& interval_time,const uint32_t& sql_mod,std::vector<std::string>& vect_sql_insert);

	void SetBacServerBalance(const std::vector<std::string>& vect_sql_insert);

private:
	MYSQL mysql_;
	MysqlConnect* connect_;
};

#endif
