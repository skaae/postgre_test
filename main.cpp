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



int main()
{
    const std::string poison_str("POISON_PILL");

    std::string body_xml("<Some xml string>");
    auto& msg = body_xml;



    rigtorp::SPSCQueue<std::unique_ptr<std::string>> xml_queue(123); 
    //rigtorp::SPSCQueue<Fill> fill_queue(123); 
    //rigtorp::SPSCQueue<Fill> order_queue(123); 
    
    auto conn0 = create_db_connection("localhost", "postgres_db", "postgres", "postgres_pw", 5433);

      // setup table.
    conn0->execute( "DROP TABLE IF EXISTS dummy_table" );
    conn0->execute( "CREATE TABLE dummy_table ( a INTEGER PRIMARY KEY, xml TEXT NOT NULL )" );

    auto xml_consumer = std::thread([&xml_queue, &conn0, poison_str] {

        conn0->prepare( "my_stmt", "INSERT INTO dummy_table VALUES ( $1, $2 )" );

        // SETUP DATABASE CONNECTION FOR THIS THREAD.
        // ...

        while (true){
            if (!xml_queue.front()){
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                std::cout << "waiting" << std::endl;
            }
            else{
                auto msg = *(xml_queue.front()->get());
                if (msg == poison_str){
                    xml_queue.front()->release();
                    xml_queue.pop();
                    break;
                }
                {
                 const auto tr = conn0->transaction();
                 tr->execute( "INSERT INTO dummy_table VALUES ( $1, $2)", 1, msg );
                 tr->commit();
                }
                // Write to db
                std::cout << msg << std::endl;
                xml_queue.front()->release();
                xml_queue.pop();
            }
        }
        std::cout << "cleanup" << std::endl;
        // cleanup db connection and die
        });


    // trader thread pushes msg on queue    
    xml_queue.push(std::make_unique<std::string>(msg));
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    /// trading ...
    // ...


    
    //  cleanup
    xml_queue.push(std::make_unique<std::string>(poison_str));
    xml_consumer.join();


    // Check result
    const auto res = conn0->execute( "SELECT * FROM dummy_table" );
    const auto v = res.vector< std::tuple< int, std::string > >();
    for (const auto vi: v){
        auto [key, xml] = vi;
        std::cout << "key=" << key << " xml=" << xml << std::endl;
    }
}