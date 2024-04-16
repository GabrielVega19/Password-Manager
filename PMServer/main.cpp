#include <boost/asio.hpp>
#include <iostream>
#include "C:/Users/jeise/source/cryptopp890/cryptlib.h"
#include "C:/Users/jeise/source/cryptopp890/sha.h"
#include <algorithm>
#include <C:/Users/jeise/source/cryptopp890/filters.h>
#include <C:/Users/jeise/source/cryptopp890/files.h>
#include <C:/Users/jeise/source/cryptopp890/hex.h>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
    using namespace CryptoPP;

    SHA1 hash;
    std::cout << "Name: " << hash.AlgorithmName() << std::endl;
    std::cout << "Digest size: " << hash.DigestSize() << std::endl;
    std::cout << "Block size: " << hash.BlockSize() << std::endl;

    HexEncoder encoder(new FileSink(std::cout));
    std::string msg = "Yoda said, Do or do not. There is no try.";
    std::string digest;

    StringSource(msg, true, new HashFilter(hash, new StringSink(digest)));

    std::cout << "Message: " << msg << std::endl;

    std::cout << "Digest: ";
    CryptoPP::StringSource(digest, true, new Redirector(encoder));
    std::cout << std::endl;

    hash.Update((const byte*)msg.data(), msg.size());
    bool verified = hash.Verify((const byte*)digest.data());

    if (verified == true)
        std::cout << "Verified hash over message" << std::endl;
    else
        std::cout << "Failed to verify hash over message" << std::endl;
    
    try {
        boost::asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 1337));

        while(true) {
            std::cout << "Accepting connections on port 1337:\n";

            tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::cout << "Client connected - Sending message\n";
            std::string hello_message = "Hello client\n";
            boost::system::error_code error;

            boost::asio::write(socket, boost::asio::buffer(hello_message), error);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
