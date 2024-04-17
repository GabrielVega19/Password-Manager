#pragma once
#include <iostream>
#include <mariadb/conncpp.hpp>
#include <memory>
#include <optional>
#include <sstream>

namespace PM{
    using std::cout;
    using std::endl;
    using std::string;
    using sqlQuery = std::unique_ptr<sql::PreparedStatement>;
    using userRecord = std::vector<std::string>;

    class DBConnection{
        public:
            DBConnection();
            userRecord queryUser(string username);
            void registerUser(string username, string password, string salt);
            std::vector<std::vector<std::string>> fetchPasswords(string username);
            void addPassword(string user, string website, string  webUsername, string webPassword, string iv);
        private:
            std::shared_ptr<sql::Driver> _driver;
            std::shared_ptr<sql::Connection> _dbConn;
    };
}