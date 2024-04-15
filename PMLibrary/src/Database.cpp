#include <PMLibrary/Database.h>

namespace PM{
    DBConnection::DBConnection(){
        sql::Driver* driver = sql::mariadb::get_driver_instance();
        sql::SQLString url("jdbc:mariadb://localhost:3306/password_manager");
        sql::Properties properties({{"user", "root"}, {"password", "T64bK&ULVWw81x88n%W7"}});
        std::unique_ptr<sql::Connection> conn(driver->connect(url, properties));

        try {
            // Create a new Statement
            std::unique_ptr<sql::Statement> stmnt(conn->createStatement());
            // Execute query
            sql::ResultSet *res = stmnt->executeQuery("select * from test");
            // Loop through and print results
            while (res->next()) {
                std::cout << "id = " << res->getInt(1);
                std::cout << ", description = " << res->getString(2);
            }
        }
        catch(sql::SQLException& e){
            std::cerr << "Error selecting tasks: " << e.what() << std::endl;
        }

        cout << "test**************************" << endl;
        
    }
}

