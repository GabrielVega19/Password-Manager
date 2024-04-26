#include <PMLibrary/Database.h>

namespace PM{
    DBConnection::DBConnection(){
        _driver = std::shared_ptr<sql::Driver>(sql::mariadb::get_driver_instance());
        sql::SQLString url("jdbc:mariadb://localhost:3306/insert");
        sql::Properties properties({{"user", "insert"}, {"password", "insert"}});
        _dbConn = std::shared_ptr<sql::Connection>(_driver->connect(url, properties));
    }

    userRecord DBConnection::queryUser(string username){
        try {
            sqlQuery stmnt(_dbConn->prepareStatement("SELECT * FROM users WHERE username = ?;"));
            stmnt->setString(1, username);
            sql::ResultSet *res = stmnt->executeQuery();
            if (res->next()) {
                std::stringstream ss;
                ss << res->getString(2);
                    
                std::stringstream ss2;
                ss2 << res->getString(3);
                userRecord result;
                result.push_back("true");
                result.push_back(ss.str());
                result.push_back(ss2.str());
                return result;
            }
            userRecord result{"false"};
            return result;
        }
        catch(sql::SQLException& e){
            std::cerr << "Error querying db while authenticating user: " << e.what() << std::endl;
            userRecord result{"false"};
            return result;
        }        
    }

    void DBConnection::registerUser(string username, string password, string salt){
        try {
            sqlQuery stmnt(_dbConn->prepareStatement("INSERT INTO users VALUES(?, ?, ?);"));
            stmnt->setString(1, username);
            
            std::stringstream ss;
            ss << password;
            stmnt->setBlob(2, &ss);
            
            std::stringstream ss2;
            ss2 << salt;
            stmnt->setBlob(3, &ss2);
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
                std::vector<std::string> entry;
                
                std::string user(res->getString(2));
                
                std::stringstream ss;
                ss << res->getString(3);
                
                std::stringstream ss2;
                ss2 << res->getString(4);

                std::stringstream ss3;
                ss3 << res->getString(5);

                std::stringstream ss4;
                ss4 << res->getString(6);

                entry.push_back(user);
                entry.push_back(ss.str());
                entry.push_back(ss2.str());
                entry.push_back(ss3.str());
                entry.push_back(ss4.str());

                retData.push_back(entry);
            }
            
            return retData;        
        }catch(sql::SQLException& e){
            std::cerr << "Error inserting into db while registering user: " << e.what() << std::endl;
            return retData;
        }
    }

    void DBConnection::addPassword(string user, string website, string  webUsername, string webPassword, string iv){
        try {
            sqlQuery stmnt(_dbConn->prepareStatement("INSERT INTO password_storage(user, website, username, password, iv) VALUES(?, ?, ?, ?, ?)"));
            stmnt->setString(1, user);

            std::stringstream ss2;
            ss2 << website;
            stmnt->setBlob(2, &ss2);

            std::stringstream ss3;
            ss3 << webUsername;
            stmnt->setBlob(3, &ss3);

            std::stringstream ss4;
            ss4 << webPassword;
            stmnt->setBlob(4, &ss4);

            std::stringstream ss5;
            ss5 << iv;
            stmnt->setBlob(5, &ss5);

            stmnt->executeQuery();
            return;
        } catch(sql::SQLException& e){
            std::cerr << "Error inserting into db while registering user: " << e.what() << std::endl;
            return;
        }        
    }
}

