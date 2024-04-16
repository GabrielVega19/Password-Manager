#include <PMLibrary/Database.h>

namespace PM{
    DBConnection::DBConnection(){
        _driver = std::shared_ptr<sql::Driver>(sql::mariadb::get_driver_instance());
        sql::SQLString url("jdbc:mariadb://localhost:3306/insertdbname");
        sql::Properties properties({{"user", "insertuser"}, {"password", "insertpassword"}});
        _dbConn = std::shared_ptr<sql::Connection>(_driver->connect(url, properties));
    }

    userRecord DBConnection::queryUser(string username){
        try {
            sqlQuery stmnt(_dbConn->prepareStatement("SELECT * FROM users WHERE username = ?;"));
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
            std::cerr << "Error querying db while authenticating user: " << e.what() << std::endl;
            userRecord result(false, "");
            return result;
        }        
    }

    void DBConnection::registerUser(string username, string password){
        try {
            sqlQuery stmnt(_dbConn->prepareStatement("INSERT INTO users VALUES(?, ?);"));
            stmnt->setString(1, username);
            stmnt->setString(2, password);
            stmnt->executeQuery();
            return;
        }
        catch(sql::SQLException& e){
            std::cerr << "Error inserting into db while registering user: " << e.what() << std::endl;
            return;
        }        
    }
}

