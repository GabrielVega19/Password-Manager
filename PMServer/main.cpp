#include <iostream>
#include <PMLibrary/Server.h>
#include <boost/asio.hpp>
#include <PMLibrary/Database.h>


int main(int, char**){
    std::cout << "creating server" << std::endl;
    PM::TCPServer server(6969);

    server.onJoin = [](PM::TCPConnection::TCPPointer tcpConn){
        std::cout << "user has joined the server: " << tcpConn->getUsername() << std::endl;
    };

    server.onLeave = [](PM::TCPConnection::TCPPointer tcpConn){
        std::cout << "user has left the server: " << tcpConn->getUsername() << std::endl;
    };

    server.onClientMsg = [](const std::string& msg, PM::TCPConnection::TCPPointer tcpConn){
        std::cout << msg << std::endl;
    };
    
    std::cout << "running server" << std::endl;
    server.run();
    
    return 0;
}
























































/*  try {
        io_context ioContext;

        tcp::acceptor acceptor(ioContext, tcp::endpoint(tcp::v4(), 6969));

        while(true) {
            cout << "Accepting connections on port 6969." << endl;

            tcp::socket socket(ioContext);
            acceptor.accept(socket);

            cout << "client connected! Sending message." << endl;
            error_code errorCode;

            write(socket, buffer("Hello client. Change to something.\n"), errorCode);
        }
    } catch (std::exception& e){
        cerr << e.what() << endl;
    }*/