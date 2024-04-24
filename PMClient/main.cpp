#include <PMLibrary/Client.h>
#include <iostream>
#include <thread>
using std::cout;
using std::endl;

int main(int argc, char* argv[]){
    PM::TCPClient client("localhost", 6969);

    client.onMessage = [](const std::string& message){
        cout << message;
    };

    std::thread t([&client] {
        client.run();
    });

    while(true){
        string msg;
        getline(std::cin, msg);

        if (msg == "quit")
            break;
        
        msg += "\n";

        client.post(msg);
    }

    client.stop();
    t.join();
    return 0;
}
