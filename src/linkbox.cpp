#include <stddef.h>
#include <cstdlib>
#include <string>

#include "Config.hpp"
#include "Network.hpp"
#include "Utils.hpp"

#define CHUNK_SIZE 4096

// TODO User authentication
// TODO File hash verification

int main(int argc, char *argv[]) {
    Config conf("/.linkbox/linkbox.conf");

    if (!conf.init())
        exit(1);

    Network net(conf.getProperty("server"), conf.getProperty("port"));

    if (!net.init())
        Utils::error("Failed to initialize connection");

    if (argc != 2)
        Utils::error("File path not given, usage: linkbox <path/to/file>");

    FILE *fp;
    fp = fopen(argv[1], "rb");

    std::string str(argv[1]);
    size_t file_name_loc = str.find_last_of("/\\");

    if (!net.sendFile(fp, str.substr(file_name_loc + 1).c_str()))
        Utils::error("Failed to complete file transfer");

    char server_response[256];
    if (!net.receive(server_response, sizeof server_response))
        Utils::error("Failed to receive server response");

    Utils::printl(server_response);

    fclose(fp);
    net.clean();

    return 0;
}

