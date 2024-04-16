#include <PMLibrary/Database.h>

namespace PM{
    DBConnection::DBConnection(){
        _driver = std::shared_ptr<sql::Driver>(sql::mariadb::get_driver_instance());
        sql::SQLString url("jdbc:mariadb://localhost:3306/insertdbname");
        sql::Properties properties({{"user", "inseruser"}, {"password", "insertpassword"}});
        _dbConn = std::shared_ptr<sql::Connection>(_driver->connect(url, properties));
    }

    userRecord DBConnection::queryUser(string username){
        try {
            sqlQuery stmnt(_dbConn->prepareStatement("select * from users where username = ?"));
            stmnt->setString(1, username);
            sql::ResultSet *res = stmnt->executeQuery();
            if (res->next()) {
                userRecord result(true, res->getString(2));
                return result;
            }
            userRecord result(false, "");
            return result;
        }
        catch(sql::SQLException& e){
            std::cerr << "Error selecting tasks: " << e.what() << std::endl;
            userRecord result(false, "");
            return result;
        }        
    }
}

