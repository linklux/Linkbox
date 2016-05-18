#include "Config.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <streambuf>
#include <sstream>

#define CONF_FILE "/.linkbox/linkbox.conf"

Config::Config() {
    char* usr_home = getenv("HOME");
    char file_loc[(sizeof(usr_home) / sizeof(char)) + (sizeof(CONF_FILE) / sizeof(char))];

    strcpy(file_loc, usr_home);
    strcat(file_loc, CONF_FILE);

    this->conf_file = file_loc;
}

bool Config::init() {
    std::ifstream f_str(this->conf_file);
    std::string str((std::istreambuf_iterator<char>(f_str)), std::istreambuf_iterator<char>());

    std::istringstream s_str(str);
    std::string line;

    while (std::getline(s_str, line)) {
        if (line.empty())
            continue;

        if (line[0] == '#')
            continue;

        std::istringstream is_line(line);
        std::string key;

        if (std::getline(is_line, key, '=')) {
            std::string value;

            if (std::getline(is_line, value)) {
                this->conf[key.c_str()] = value.c_str();

                printf("[DEBUG] %s: %s\n", key.c_str(), value.c_str());
            }
        }
    }

    return true;
}

std::string Config::getProperty(std::string key) {
    if (this->conf.find(key) == this->conf.end())
        return NULL;

    return this->conf[key];
}
