#include <PMLibrary/Server.h>
#include <iostream>

namespace PM{
    TCPServer::TCPServer(int port): _port(port), _acceptor(_ioContext, tcp::endpoint(tcp::v4(), _port)) {
        _dbConnection = std::shared_ptr<DBConnection>(new DBConnection());

        _ctx.use_certificate_chain_file("/home/gabriel/Desktop/School-Shit/CN-Security/Password-Manager/certs/passwordmanager.crt");
        _ctx.use_private_key_file("/home/gabriel/Desktop/School-Shit/CN-Security/Password-Manager/certs/passwordmanager.pem", boost::asio::ssl::context::pem);
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
        _socket.emplace(_ioContext, _ctx);

        _acceptor.async_accept((*_socket).lowest_layer(), [this](const error_code& error){
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



    TCPConnection::TCPConnection(ssl_socket&& socket, DBPointer dbConnection) : _socket(std::move(socket)), _dbConnection(dbConnection){
        error_code ec;
        std::stringstream name;
        name << _socket.lowest_layer().remote_endpoint();

        _name = name.str();
    }
    
    void TCPConnection::start(msgHandler&& msgHandler, errHandler&& errHandler){
        _socket.async_handshake(boost::asio::ssl::stream_base::server, [](const boost::system::error_code &error){
            if (error){
                cout << error.what() << endl;
            }
        });
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
            
            // prompts user to reenter password for registration
            send("Please enter your password again to register yourself in the system.\n\n");
            string repeatPassword = syncRead();
            
            // ensures passwords match
            CryptoPP::SHA256 hash;
            repeatPassword = repeatPassword + _salt;
            hash.Update((const CryptoPP::byte*)repeatPassword.data(), repeatPassword.size());
            bool verified = hash.Verify((const CryptoPP::byte*)_password.data());
            
            if (verified){
                // if they match then it registers the user 
                registerUser();
                cout << "Registered User." << endl;
                send("Registration Successful. Welcome to the Secure Password Manager.\n");
                serviceClient();
            }else{
                // if they dont then close connection 
                send("Passwords dont match please connect to the server and try again.");
                _socket.lowest_layer().close();
                _errHandler();
                return;
            }

        }else if (authenticationStatus == "false"){
            //dissconect the user for failing to authenticate to a registered user
            send("Authentication failed.");
            cout << "Authentication Failed." << endl;
            _socket.lowest_layer().close();
            _errHandler();
            return;
        }else{
            //throw an error something went wrong
            _socket.lowest_layer().close();
            _errHandler();
            return;
        }

    }

    void TCPConnection::registerUser(){
        _dbConnection->registerUser(_username, _password, _salt);
    }

    string TCPConnection::authenticateUser(){        
        // prepares the hashing algorithm code 
        CryptoPP::SHA256 hash;
        std::string hashedPassword;
        hashedPassword.resize(hash.DigestSize());

        // reads in two strings 
        string userUsername = syncRead();
        string userPassword = syncRead();

        // sets username 
        _username = userUsername;
       
        userRecord result = _dbConnection->queryUser(userUsername);
        if (result[0] == "true"){
            // sets salt in tcpConnection
            _salt = result[2];
            // salts and hashes password and sets in tcp object
            userPassword = userPassword + _salt;
            hash.Update((const CryptoPP::byte*)userPassword.data(), userPassword.size());
            hash.Final((CryptoPP::byte*)&hashedPassword[0]);
            _password = hashedPassword;

            // ensures passwords match
            hash.Update((const CryptoPP::byte*)userPassword.data(), userPassword.size());
            bool verified = hash.Verify((const CryptoPP::byte*)result[1].data());
            if (verified){
                _aesKey = deriveAESKey(userPassword, _salt);

                return "true";
            }else{
                return "false";
            }
        }else{
            // generate a salt and set it in the tcp server
            const unsigned int BLOCKSIZE = 256;
            CryptoPP::SecByteBlock scratch( BLOCKSIZE );
            CryptoPP::AutoSeededRandomPool rng;
            rng.GenerateBlock( scratch, scratch.size() );
            string salt = std::move((char*)&scratch);
            _salt = salt;

            // salts and hashes password and sets in tcp object
            userPassword = userPassword + _salt;
            hash.Update((const CryptoPP::byte*)userPassword.data(), userPassword.size());
            hash.Final((CryptoPP::byte*)&hashedPassword[0]);
            _password = hashedPassword;

            _aesKey = deriveAESKey(userPassword, _salt);
            return "register";
        }
    }

    CryptoPP::SecByteBlock TCPConnection::deriveAESKey(string pass, string salt){
        // derives the key for AES encryption of the user data
        // sets up the password and salt in the format needed 
        byte* testPassword = (byte*)pass.data();
        size_t plen = strlen((const char*)testPassword);
        byte* testSalt = (byte*)salt.data();
        size_t slen = strlen((const char*)testSalt);

        // sets up the memory for the derived key
        byte testDerived[CryptoPP::SHA256::DIGESTSIZE];

        // creates the object to derive the key
        CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf;
        byte unused = 0;

        // derives the key
        pbkdf.DeriveKey(testDerived, sizeof(testDerived), unused, testPassword, plen, testSalt, slen, 1024, 0.0f);

        // converts the derived bytes for the key into a string 
        return CryptoPP::SecByteBlock(testDerived, CryptoPP::SHA256::DIGESTSIZE);
        // CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(derivedKey));
        // encoder.Put(testDerived, sizeof(testDerived));
        // encoder.MessageEnd();
    }

    string TCPConnection::syncRead(){
        error_code ec;
        boost::asio::read_until(_socket, _streamBuff, "\n");
        
        std::stringstream msg;
        msg << std::istream(&_streamBuff).rdbuf();
        std::string stringifyMsg = msg.str();

        return stringifyMsg.substr(0, (stringifyMsg.size() - 1));
    }

    void TCPConnection::serviceClient(){       
        send("- Type 1 and then enter to add a password.\n- Type 2 and then enter to fetch passwords.\n\n");
        
        string msg = syncRead();
        onRead(msg);
    }

    string TCPConnection::encrypt(string plaintext, CryptoPP::SecByteBlock iv){
        std::string cipher;
        encryption e;
        e.SetKeyWithIV(_aesKey, _aesKey.size(), iv);
        CryptoPP::StringSource s(plaintext, true, new CryptoPP::StreamTransformationFilter(e, new CryptoPP::StringSink(cipher)));

        return cipher;
    }

    string TCPConnection::decrypt(string ciphertext, CryptoPP::SecByteBlock iv){
        std::string plaintext;
        decryption d;
        d.SetKeyWithIV(_aesKey, _aesKey.size(), iv);
        CryptoPP::StringSource s(ciphertext, true, new CryptoPP::StreamTransformationFilter(d, new CryptoPP::StringSink(plaintext)));

        return plaintext;
    }

    void TCPConnection::onRead(string msg){        
        string stringifyMsg = msg;
        _msgHandler(_username + ": " + stringifyMsg);

        if (stringifyMsg == "1"){ 
            cout << "inside 1" << endl;
            // generates an IV for encrypting the record
            CryptoPP::AutoSeededRandomPool prng;
            CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);
            prng.GenerateBlock(iv, iv.size());
            string ivString(reinterpret_cast<const char*>(iv.data()), iv.size());

            send("Website Link (ex. google.com): \n"); 
            string website = syncRead();
            send("Username (ex. gabriel@gmail.com): \n"); 
            string webUsername = syncRead();
            send("Password (ex. mypassword): \n");
            string webPassword = syncRead();

            
            _dbConnection->addPassword(_username, encrypt(website, iv), encrypt(webUsername, iv), encrypt(webPassword, iv), ivString);
            send("Password Successfully registered.\n");

        } else if(stringifyMsg == "2"){           
            std::vector<std::vector<std::string>> passwords = _dbConnection->fetchPasswords(_username);
            for (auto entry : passwords){                
                CryptoPP::SecByteBlock iv(reinterpret_cast<const byte*>(&entry[4][0]), entry[4].size());

                std::stringstream msgStream;
                msgStream << "-" << decrypt(entry[1], iv) << std::endl; 
                msgStream << "\t-username: " << decrypt(entry[2], iv) << std::endl;
                msgStream << "\t-password: " << decrypt(entry[3], iv) << std::endl;
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