#include "easylogging++.h"
#include <iostream>
#include "db_mysql.h"
#include "common.h"
#include <time.h>
#include <typeinfo>
#include <sys/timeb.h>
#include <stdlib.h>
#include <boost/program_options.hpp>
using namespace  boost::program_options;
INITIALIZE_EASYLOGGINGPP
static uint64_t g_offset = 0;
static ConfManager g_conf;
/*
#include<boost/asio/io_service.hpp>
#include<boost/asio/ip/tcp.hpp>
#include<boost/bind.hpp>
#include<boost/shared_ptr.hpp>
#include<boost/enable_shared_from_this.hpp>
#include<string>
#include<iostream>
#include<boost/asio/streambuf.hpp>
#include<boost/asio/placeholders.hpp>
#include<boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::address;

class client : public boost::enable_shared_from_this<client> {
    public:
        client(boost::asio::io_service &io_service, tcp::endpoint &endpoint)
            : io_service_(io_service), socket_(io_service), endpoint_(endpoint)
        {
        }

        void start() {
            socket_.async_connect(endpoint_,
                    boost::bind(&client::handle_connect,
                        shared_from_this(),
                        boost::asio::placeholders::error));
        }

    private:
        void handle_connect(const boost::system::error_code &error) {
            if (error) {
                if (error.value() != boost::system::errc::operation_canceled) {
                    std::cerr << boost::system::system_error(error).what() << std::endl;
                }

                socket_.close();
                return;
            }

            static tcp::no_delay option(true);
            socket_.set_option(option);

            strcpy(buf, "{\"id\":1000, \"method\": \"kline.subscribe\", \"params\":[\"BACCNY\",10] }");
            boost::asio::async_write(socket_,
                    boost::asio::buffer(buf, strlen(buf)),
                    boost::bind(&client::handle_write,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }

        void handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
            memset(buf, sizeof(buf), 0);
            boost::asio::async_read_until(socket_,  sbuf,"\n",
                    boost::bind(&client::handle_read,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
           std::cout << "handle writer :" << std::endl;
        }

        void handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
            std::cout << "handle read: " <<buf << std::endl;
        }

    private:
        boost::asio::io_service &io_service_;
        tcp::socket socket_;
        tcp::endpoint &endpoint_;
        char buf[1024];
        boost::asio::streambuf sbuf;
};

typedef boost::shared_ptr<client> client_ptr;

int main(int argc, char* argv[])
{
    boost::asio::io_service io_service;
    tcp::endpoint endpoint(address::from_string("47.99.82.55"), 8099);

    client_ptr new_session(new client(io_service, endpoint));
    new_session->start();
    io_service.run();
    std::string ss;
    std::cin >> ss;
    return 0;
}



static void TcpClient()
{
    try
    {
        boost::asio::io_service io_service;
        tcp::endpoint end_point(boost::asio::ip::address::from_string("47.99.82.55"), 8099);
        tcp::socketsocket(io_service);
        socket.connect(end_point);
        for (;;)
        {
            boost::array<char, 1024> buf;
            boost::system::error_code error;

            size_t len = socket.read_some(boost::asio::buffer(buf), error);
            socket.write_some();
            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error); // Some other error.

            std::cout.write(buf.data(), len);
        }
     }
     catch (std::exception& e)
     {
         std::cerr << e.what() << std::endl;
     }
    
}
*/


void DealWithData()
{
		DBMysql::MysqlConnect* connect_bac = new DBMysql::MysqlConnect();
		connect_bac->use_db = g_conf.getArgs("mysqldb","api_tran");
		connect_bac->user_pass = g_conf.getArgs("mysqlpass","a");
		connect_bac->port = std::atoi(g_conf.getArgs("mysqlport","3306").c_str());
		connect_bac->user_name = g_conf.getArgs("mysqluser","snort");
		connect_bac->url = g_conf.getArgs("mysqlserver","192.168.0.18");
		DBMysql db_bac;
		db_bac.SetConnect(connect_bac);
		db_bac.OpenDB();
		uint32_t sql_mod = 100;
		while (true)
		{
			struct timeb tp;
			ftime(&tp);
			uint64_t current = tp.time;

			uint64_t compute_time = (current/3600) * 3600  - rand()%(10*60) -60*30;
			std::cout <<  current << ":" << compute_time << std::endl;
			std::vector<std::string> vect_sql_insert;
			db_bac.GetExchangeBalance(compute_time,sql_mod, vect_sql_insert);
			struct timeb tp_select;
			ftime(&tp_select);
			std::cout << tp_select.time << std::endl;

			db_bac.SetBacServerBalance(vect_sql_insert);
			current = tp.time;
			struct timeb tp_end;
			ftime(&tp_end);
			std::cout << tp_end.time << std::endl;
			break;
		}
		db_bac.CloseDB();

}


int main(int argc,char*argv[])
{

    std::string path ; // 外部变量 存储 参数path的值
    std::string rpc_url;
    std::vector<std::string> rpc_params;
    boost::program_options::options_description opts("All options");
    opts.add_options()
        ("help,h","help info")
        ("configure_path,c",value<std::string>(&path)->default_value("../conf/server_main.conf"),"path configure ")
        ("rpc_url,u",value<std::string>(&rpc_url)->default_value("http://127.0.0.1:8080"),"rpc url")
        ("rpc_function,f",value<std::string>(),"rpc function") // 
        ("rpc_param,p",value<std::vector<std::string> >(&rpc_params)->multitoken(),"rpc function params"); //多个参数

    variables_map vm;
    try
    {
        store(parse_command_line(argc,argv,opts),vm); // 分析参数
    }
    catch(boost::program_options::error_with_no_option_name &ex)
    {
        std::cout<<ex.what()<<std::endl;
    }

    notify(vm); // 将解析的结果存储到外部变量
    if (vm.count("help"))
    {
        std::cout<<opts<<std::endl;
        return -1;
    }
    if(vm.count("path"))
    {
        std::cout<<vm["path"].as<std::string>()<< std::endl;
    }

    std::cout<<path<<std::endl;
    std::cout<<rpc_params.size()<<std::endl;
    daemon(1,1);
	g_conf.setConfPath(path);
	g_conf.readConfigFile();
	g_conf.printArg();
    std::string log_path = g_conf.getArgs("log_path","../conf/server_log.conf");
    el::Configurations conf(log_path);
    el::Loggers::reconfigureAllLoggers(conf);

//	DealWithData();
}
