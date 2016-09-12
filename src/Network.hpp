#ifndef NETWORK_HPP_
#define NETWORK_HPP_

#include <string>

class Network {

    public:
        Network(std::string server, std::string port);

        bool init();
        void clean();

        bool sendData(const char* buf, size_t byte_size);
        bool sendHeader(const char* file_name, size_t file_size);
        bool sendFile(const char* file_name);

        bool receive(char* buf, size_t byte_size);

        char* err() {
            return this->error;
        }

        int getSockfd() {
            return this->sockfd;
        }

    private:
        bool validate();

        int sockfd = 0;
        char* error;

        std::string server, port;

};

#endif
