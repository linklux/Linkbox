#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "Config.hpp"
#include "Network.hpp"

#define CHUNK_SIZE 4096

// TODO User authentication
// TODO File hash verification

void debug(const char* msg, bool terminate) {
    printf("[DEBUG] %s\n", msg);

    if (terminate)
        exit(1);
}

int main(int argc, char *argv[]) {
    Config conf;

    if (!conf.init())
        debug("Failed to read config file", true);

    Network net(conf.getProperty("server"), conf.getProperty("port"));

    if (!net.init())
        debug("Failed to initialize connection", true);

    if (argc != 2)
        debug("File path not given, usage: linkbox <path/to/file>", true);

    FILE *fp;
    fp = fopen(argv[1], "rb");

    std::string str(argv[1]);
    size_t file_name_loc = str.find_last_of("/\\");

    if (!net.sendFile(fp, str.substr(file_name_loc + 1).c_str()))
        debug("Failed to complete file transfer", true);

    char server_response[256];
    if (!net.receive(server_response, sizeof server_response))
        debug("Failed to receive server response", true);

    printf("%s\n", server_response);

    fclose(fp);
    net.clean();

    return 0;
}

