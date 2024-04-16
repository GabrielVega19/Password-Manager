#include <PMLibrary/Server.h>
#include <iostream>

namespace PM{
    TCPServer::TCPServer(int port): _port(port), _acceptor(_ioContext, tcp::endpoint(tcp::v4(), _port)) {
        _dbConnection = std::shared_ptr<DBConnection>(new DBConnection());
    }

    int TCPServer::run(){
        try{
            startAccept();
            _ioContext.run();
        }catch(std::exception& e){
            std::cerr << e.what() << std::endl;
            return -1;
        }
        return 0;
    }

    void TCPServer::startAccept(){
        _socket.emplace(_ioContext);

        _acceptor.async_accept(*_socket, [this](const error_code& error){
            auto conn = TCPConnection::create(std::move(*_socket), _dbConnection);

            if (onJoin){
                onJoin(conn);
            }

            _connections.insert(conn);

            if (!error){
                conn->start(
                    [this, conn](const std::string& msg){
                        if (onClientMsg){
                            onClientMsg(msg, conn);
                        }
                    },
                    [&, weak = std::weak_ptr(conn)](){
                        if (auto shared = weak.lock(); shared && _connections.erase(shared)){
                            if (onLeave) onLeave(shared);
                        }
                    }
                );
            }

            startAccept();
        });
    }



    TCPConnection::TCPConnection(tcp::socket&& socket, DBPointer dbConnection) : _socket(std::move(socket)), _dbConnection(dbConnection){
        error_code ec;
        std::stringstream name;
        name << _socket.remote_endpoint();

        _name = name.str();
    }
    
    void TCPConnection::start(msgHandler&& msgHandler, errHandler&& errHandler){
        _msgHandler = std::move(msgHandler);
        _errHandler = std::move(errHandler);

        //attempt to authenticate the user
        cout << "authenticating user" << endl;
        string authenticationStatus = authenticateUser();
        if (authenticationStatus == "true"){
            //if authenticated service client
            cout << "passed authentication" << endl;
            serviceClient();
        }else if (authenticationStatus == "register"){
            // if user not registered in db then register them in the db 
            cout << "need to register" << endl;
        }else if (authenticationStatus == "false"){
            //dissconect the user for failing to authenticate to a registered user
            cout << "failed authentication" << endl;
            _socket.close();
            _errHandler();
            return;
        }else{
            //throw an error something went wrong
            _socket.close();
            _errHandler();
            return;
        }

    }

    string TCPConnection::authenticateUser(){
        string userUsername = syncRead();
        string userPassword = syncRead();

        userRecord result = _dbConnection->queryUser(userUsername);
        if (result.first){
            if (result.second == userPassword){
                return "true";
            }else{
                return "false";
            }
        }else{
            return "register";
        }
    }

    string TCPConnection::syncRead(){
        error_code ec;
        boost::asio::read_until(_socket, _streamBuff, "\n");
        
        std::stringstream msg;
        msg << std::istream(&_streamBuff).rdbuf();
        std::string stringifyMsg = msg.str();

        return stringifyMsg.substr(0, (stringifyMsg.size() - 2));
    }


    void TCPConnection::serviceClient(){
        boost::asio::async_read_until(_socket, _streamBuff, "\n", [self=shared_from_this(), this](error_code ec, size_t bLen){            
            self->onRead(ec, bLen);
        });
    }
    void TCPConnection::onRead(error_code ec, size_t bLen){
        if (ec){
            _socket.close();

            _errHandler();
            return;
        }

        std::stringstream msg;
        msg << _name << ": " << std::istream(&_streamBuff).rdbuf();

        _msgHandler(msg.str());
        serviceClient();
    };

    void TCPConnection::send(const std::string& msg){
        bool queueIdle = _outgoingMsgs.empty();
        _outgoingMsgs.push(msg);

        if (queueIdle){
            asyncWrite();
        }
    }
    
    void TCPConnection::asyncWrite(){
        boost::asio::async_write(_socket, boost::asio::buffer(_outgoingMsgs.front()), [self=shared_from_this(), this](error_code ec, size_t bLen){            
            self->onWrite(ec, bLen);
        });
    }

    void TCPConnection::onWrite(error_code ec, size_t bLen){
        if (ec){
            _socket.close();
            _errHandler();
            return;
        }

        _outgoingMsgs.pop();
        if (!_outgoingMsgs.empty()){
            asyncWrite();
        }
    }
}