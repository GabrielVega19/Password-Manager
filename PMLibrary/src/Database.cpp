#include <PMLibrary/Database.h>

namespace PM{
    DBConnection::DBConnection(){
        _driver = std::shared_ptr<sql::Driver>(sql::mariadb::get_driver_instance());
        sql::SQLString url("jdbc:mariadb://localhost:3306/insertdbname");
        sql::Properties properties({{"user", "insertusername"}, {"password", "insertpassword"}});
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
        } catch(sql::SQLException& e){
            std::cerr << "Error inserting into db while registering user: " << e.what() << std::endl;
            return;
        }        
    }

    std::vector<std::vector<std::string>> DBConnection::fetchPasswords(string username){
        std::vector<std::vector<std::string>> retData;

        try {
            sqlQuery stmnt(_dbConn->prepareStatement("SELECT * FROM password_storage WHERE user = ?;"));
            stmnt->setString(1, username);
            sql::ResultSet *res = stmnt->executeQuery();
            while (res->next()) {
                cout << res->getString(2) << username << endl;
                std::vector<std::string> entry;

                std::string user(res->getString(1));
                std::string website(res->getString(2));
                std::string webUsername(res->getString(3));
                std::string webPassword(res->getString(4));
                entry.push_back(user);
                entry.push_back(website);
                entry.push_back(webUsername);
                entry.push_back(webPassword);

                retData.push_back(entry);
            }
            
            return retData;        
        }catch(sql::SQLException& e){
            std::cerr << "Error inserting into db while registering user: " << e.what() << std::endl;
            return retData;
        }
    }

    void DBConnection::addPassword(string user, string website, string  webUsername, string webPassword){
        try {
            sqlQuery stmnt(_dbConn->prepareStatement("INSERT INTO password_storage VALUES(?, ?, ?, ?);"));
            stmnt->setString(1, user);
            stmnt->setString(2, website);
            stmnt->setString(3, webUsername);
            stmnt->setString(4, webPassword);
            stmnt->executeQuery();
            return;
        } catch(sql::SQLException& e){
            std::cerr << "Error inserting into db while registering user: " << e.what() << std::endl;
            return;
        }        
    }
}

