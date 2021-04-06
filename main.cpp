#include <cassert>
#include <iostream>
#include <thread>
#include <string>
#include <chrono>

#include <tao/pq.hpp>
#include "SPSCQueue.h"
// 

auto create_db_connection(std::string host, std::string db_name, std::string user, std::string password, int port){
    std::string db_str = "host=";
    db_str += host;
    db_str += " dbname=";
    db_str += db_name;
    db_str += " user=";
    db_str += user;
    db_str += " password=";
    db_str += password;
    db_str += " port=";
    db_str += std::to_string(port);
    

    const auto conn = tao::pq::connection::create( db_str );
    return conn;
}


// struct timed_xml_log{
//     std::unique_ptr<std::string>> xml;
    
// };


//  t = absl::FromUnixMicros(-1);
//   EXPECT_EQ("1969-12-31 23:59:59.999999",
//             absl::FormatTime("%Y-%m-%d %H:%M:%E*S", t, tz));


auto log_xml( rigtorp::SPSCQueue<std::unique_ptr<std::string>>& queue, std::shared_ptr<tao::pq::connection> conn, std::string poison){
        conn->prepare( "my_stmt", "INSERT INTO xml_stream VALUES ( $1, $2, $3 )" );
        while (true){
            if (!queue.front()){
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                std::cout << "waiting" << std::endl;
            }
            else{
                auto msg = *(queue.front()->get());
                if (msg == poison){
                    queue.front()->release();
                    queue.pop();
                    break;
                }
                {
                 const auto tr = conn->transaction();
                 tr->execute( "my_stmt", "1999-12-31", "1969-12-31 23:59:59.999999", msg );
                 tr->commit();
                }
                // Write to db
                std::cout << msg << std::endl;
                queue.front()->release();
                queue.pop();
            }
        }
        std::cout << "cleanup" << std::endl;
}


//INSERT INTO foo VALUES ( to_timestamp(1342986162) )
int main()
{

    auto port_str = std::getenv("POSTGRES_PORT");
    int port = 5433;
    if (port_str){
        port = std::stoi(port_str);
    }

    const std::string poison("POISON_PILL");

    std::string body_xml("<Some xml string>");
    auto& msg = body_xml;

    // rigtorp::SPSCQueue<std::unique_ptr<std::string>> xml_queue();
    // auto logger_tup = create_xml_queue("localhost", "postgres_db", "postgres", "postgres_pw", 5433);
    // PGPASSWORD=postgres_pw psql -h localhost -p 5433 -U postgres -f xml_stram.sql

    rigtorp::SPSCQueue<std::unique_ptr<std::string>> queue(123);   
    auto conn = create_db_connection("localhost", "postgres_db", "postgres", "postgres_pw", port);



      // setup table.
    //conn->execute( "DROP TABLE IF EXISTS xml_stream" );
    //conn->execute( "CREATE TABLE xml_stream ( local_trading_date DATE NOT NULL, timestamp TIMESTAMP NOT NULL, xml TEXT NOT NULL)" );
    //conn->execute( "SELECT create_hypertable('xml_stream' , 'local_trading_date',  chunk_time_interval => interval '1 day')" );
    // SELECT create_hypertable('event_hyper', 'id', chunk_time_interval => 1000000);

    auto xml_consumer = std::thread([&queue, &conn, poison] { log_xml(queue, conn, poison); });


    // thread pushes msg on queue    
    queue.push(std::make_unique<std::string>(msg));

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
 
    //  cleanup
    queue.push(std::make_unique<std::string>(poison));
    xml_consumer.join();


    // Check result
    const auto res = conn->execute( "SELECT * FROM xml_stream" );
    const auto v = res.vector< std::tuple<std::string, std::string, std::string > >();
    for (const auto vi: v){
        auto [date, ts, xml] = vi;
        std::cout  << "date=" << date << "  ts=" << ts << " xml=" << xml << std::endl;
    }

    return 0;
}
