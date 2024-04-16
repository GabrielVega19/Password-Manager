#pragma once
#include <iostream>
#include <mariadb/conncpp.hpp>
#include <memory>
#include <optional>

namespace PM{
    using std::cout;
    using std::endl;
    using std::string;
    using sqlQuery = std::unique_ptr<sql::PreparedStatement>;
    using userRecord = std::pair<bool, std::string>;

    class DBConnection{
        public:
            DBConnection();
            userRecord queryUser(string username);

        private:
            std::shared_ptr<sql::Driver> _driver;
            std::shared_ptr<sql::Connection> _dbConn;
    };
}