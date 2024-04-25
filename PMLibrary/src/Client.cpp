#include <PMLibrary/Client.h>

namespace PM{
    TCPClient::TCPClient(const string& address, int port): _socket(_ioContext, _ctx){

        tcp::resolver resolver{_ioContext};
        _endpoints = resolver.resolve(address, std::to_string(port));
    }

    void TCPClient::run(){
        boost::asio::async_connect(_socket.lowest_layer(), _endpoints, [this](error_code ec, tcp::endpoint ep){
            _socket.handshake(ssl_socket::client);


            if (!ec){
                asyncRead();
            }
        });
        cout << "Welcome to the password manager! Please enter your username and password." << endl;
        _ioContext.run();
    }

    void TCPClient::stop(){
        error_code ec;

        _socket.lowest_layer().close(ec);
    }

    void TCPClient::post(const string& message){
        bool queueIdle = _outgoingMessages.empty();
        _outgoingMessages.push(message);

        if (queueIdle){
            asyncWrite();
        }
    }

    void TCPClient::asyncRead(){
        boost::asio::async_read_until(_socket, _streamBuf, "\n", [this](error_code ec, size_t bytes){
            onRead(ec, bytes);
        });
    }

    void TCPClient::onRead(error_code ec, size_t bytes){
        if (ec){
            stop();
            return;
        }

        std::stringstream message;
        message << std::istream{&_streamBuf}.rdbuf();
        onMessage(message.str());
        asyncRead();
    }

    void TCPClient::asyncWrite(){ 
        boost::asio::write(_socket, boost::asio::buffer(_outgoingMessages.front()));
        onWrite();
    }

    void TCPClient::onWrite(){
        _outgoingMessages.pop();

        if (!_outgoingMessages.empty()){
            asyncWrite();
        }
    }
}