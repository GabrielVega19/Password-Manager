#pragma once
#include <iostream>
#include <memory>
#include <optional>
#include <functional>
#include <queue>
#include <unordered_set>
#include <boost/asio.hpp>
#include <PMLibrary/Database.h>


namespace PM{
    using boost::asio::ip::tcp;
    using boost::system::error_code;
    using std::cout;
    using std::endl;
    using msgHandler = std::function<void(std::string)>;
    using errHandler = std::function<void()>;

    class TCPConnection : public std::enable_shared_from_this<TCPConnection>{
        public:
            using TCPPointer = std::shared_ptr<TCPConnection>;

            static TCPPointer create(tcp::socket&& socket){
                return TCPPointer(new TCPConnection(std::move(socket)));
            }
            
            tcp::socket& socket(){
                return _socket;
            }
            void start(msgHandler&& msgHandler, errHandler&& errHandler);
            void send(const std::string& msg);
            inline const std::string& getUsername() const { return _name; }
        private:
            tcp::socket _socket;
            std::string _name; 
            std::queue<std::string> _outgoingMsgs;
            boost::asio::streambuf _streamBuff {65536};
            msgHandler _msgHandler;
            errHandler _errHandler;

            explicit TCPConnection(tcp::socket&& socket);
            void asyncRead();
            void onRead(error_code ec, size_t bLen);
            void asyncWrite();
            void onWrite(error_code ec, size_t bLen);
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
            tcp::acceptor _acceptor; 
            std::optional<tcp::socket> _socket;
            std::unordered_set<TCPConnection::TCPPointer> _connections {};
            DBConnection _dbConnection;
            
            void startAccept();
    };

}