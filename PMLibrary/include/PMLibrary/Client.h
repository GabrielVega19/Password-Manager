#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <queue>

using std::string;
using boost::system::error_code;
using MessageHandler = std::function<void(string)>;
using boost::asio::io_context;
using boost::asio::ip::tcp;
using std::cout;
using std::endl;

namespace PM{

    class TCPClient{
        public:
            MessageHandler onMessage;

            TCPClient(const string& address, int port);

            void run();
            void stop();
            void post(const string& message);

        private:
            io_context _ioContext{};
            tcp::socket _socket;
            tcp::resolver::results_type _endpoints;
            boost::asio::streambuf _streamBuf{65536};
            std::queue<string> _outgoingMessages{};

            void asyncRead();
            void onRead(error_code ec, size_t btyes);
            void asyncWrite();
            void onWrite();
    };
};