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
            cout << "user authenticated" << endl;
            send("Authentication Successful. Welcome to the Secure Password Manager.\n");
            serviceClient();
        }else if (authenticationStatus == "register"){
            // if user not registered in db then register them in the db 
            cout << "Registering User" << endl;
            send("Please enter your password again to register yourself in the system.\n\n");
            string repeatPassword = syncRead();
            
            if (repeatPassword == _password){
                registerUser();
                cout << "Registered User." << endl;
                send("Registration Successful. Welcome to the Secure Password Manager.\n");
                serviceClient();
            }else{
                send("Passwords dont match please connect to the server and try again.");
                _socket.close();
                _errHandler();
                return;
            }

        }else if (authenticationStatus == "false"){
            //dissconect the user for failing to authenticate to a registered user
            send("Authentication failed.");
            cout << "Authentication Failed." << endl;
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

    void TCPConnection::registerUser(){
        _dbConnection->registerUser(_username, _password);
    }

    string TCPConnection::authenticateUser(){
        string userUsername = syncRead();
        string userPassword = syncRead();
        _username = userUsername;
        _password = userPassword;

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
        send("- Type 1 and then enter to add a password.\n- Type 2 and then enter to fetch passwords.\n\n");

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
        msg << std::istream(&_streamBuff).rdbuf();
        std::string stringifyMsg = msg.str();
        stringifyMsg = stringifyMsg.substr(0, (stringifyMsg.size() - 2));
        _msgHandler(_username + ": " + stringifyMsg);

        if (stringifyMsg == "1"){ 
            send("Website Link (ex. google.com): "); 
            string website = syncRead();
            send("Username (ex. gabriel@gmail.com): "); 
            string webUsername = syncRead();
            send("Password (ex. mypassword): ");
            string webPassword = syncRead();

            _dbConnection->addPassword(_username, website, webUsername, webPassword);
            send("Password Successfully registered.\n");

        } else if(stringifyMsg == "2"){
            std::vector<std::vector<std::string>> passwords = _dbConnection->fetchPasswords(_username);
            for (auto entry : passwords){
                std::stringstream msgStream;
                msgStream << "-" << entry[1] << std::endl; 
                msgStream << "\t-username: " << entry[2] << std::endl;
                msgStream << "\t-password: " << entry[3] << std::endl;
                send(msgStream.str()); 
            }
        }else{
            send("Invalid Option Selected.\n");
        }

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
        boost::asio::write(_socket, boost::asio::buffer(_outgoingMsgs.front()));
        onWrite();
    }

    void TCPConnection::onWrite(){
        _outgoingMsgs.pop();
        if (!_outgoingMsgs.empty()){
            asyncWrite();
        }
    }
}