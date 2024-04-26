#pragma once
#include <iostream>
#include <memory>
#include <optional>
#include <functional>
#include <queue>
#include <unordered_set>
#include <boost/asio.hpp>
#include <PMLibrary/Database.h>
#include <cryptlib.h>
#include <sha.h>
#include <cryptopp/osrng.h>
#include <pwdbased.h>
#include <hex.h>
#include <cryptopp/rijndael.h>
#include <cryptopp/modes.h>
#include <boost/asio/ssl.hpp>

namespace PM{
    using boost::asio::ip::tcp;
    using boost::system::error_code;
    using std::cout;
    using std::endl;
    using std::string;
    using CryptoPP::byte;
    using msgHandler = std::function<void(std::string)>;
    using errHandler = std::function<void()>;
    using encryption = CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption;
    using decryption = CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption;
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

    class TCPConnection : public std::enable_shared_from_this<TCPConnection>{
        public:
            using TCPPointer = std::shared_ptr<TCPConnection>;
            using DBPointer = std::shared_ptr<DBConnection>;

            static TCPPointer create(ssl_socket&& socket, DBPointer dbConnection){
                return TCPPointer(new TCPConnection(std::move(socket), dbConnection));
            }
            
            void start(msgHandler&& msgHandler, errHandler&& errHandler);
            void send(const std::string& msg);
            inline const std::string& getUsername() const { return _name; }
        private:
            ssl_socket _socket;
            std::string _name;
            std::string _username;
            std::string _password; 
            CryptoPP::SecByteBlock _aesKey;
            std::string _salt;
            std::queue<std::string> _outgoingMsgs;
            boost::asio::streambuf _streamBuff {65536};
            msgHandler _msgHandler;
            errHandler _errHandler;
            DBPointer _dbConnection;

            explicit TCPConnection(ssl_socket&& socket, DBPointer dbConnection);
            string syncRead();
            CryptoPP::SecByteBlock deriveAESKey(string pass, string salt);
            string encrypt(string plaintext, CryptoPP::SecByteBlock iv);
            string decrypt(string ciphertext, CryptoPP::SecByteBlock iv);
            void serviceClient();
            void registerUser();
            void onRead(string msg);
            void asyncWrite();
            void onWrite();
            string authenticateUser();
    };

    class TCPServer{
        public:
            using onJoinHandler = std::function<void(TCPConnection::TCPPointer)>;
            using onLeaveHandler = std::function<void(TCPConnection::TCPPointer)>;
            using onClientMsgHandler = std::function<void(std::string, TCPConnection::TCPPointer)>;

            TCPServer(int port);
            int run();
            onJoinHandler onJoin;
            onLeaveHandler onLeave;
            onClientMsgHandler onClientMsg;


        private:
            int _port;
            boost::asio::io_context _ioContext;
            boost::asio::ssl::context _ctx{boost::asio::ssl::context::sslv23};
            tcp::acceptor _acceptor; 
            std::optional<ssl_socket> _socket;
            std::unordered_set<TCPConnection::TCPPointer> _connections {};
            std::shared_ptr<DBConnection> _dbConnection;
            
            void startAccept();
    };

}