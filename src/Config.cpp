#include "Config.hpp"

#include <cstring>

#include <fstream>
#include <sstream>

#include "Utils.hpp"

Config::Config(const char * file) {
    const char* usr_home = getenv("HOME");
    char file_loc[sizeof(usr_home) + sizeof(file)];

    strcpy(file_loc, usr_home);
    strcat(file_loc, file);

    this->conf_file.assign(file_loc);
}

bool Config::init() {
    if (!Utils::hasPermissions(this->conf_file.c_str(), Utils::P_READ)) {
        Utils::error("Configuration file not found or insufficient permissions");
        return false;
    }

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

            if (std::getline(is_line, value))
                this->conf[key.c_str()] = value.c_str();
        }
    }

    Utils::setDebugMode(this->getProperty("debug") == "true" ? true : false);

    for (auto& it : conf)
        Utils::debug((it.first + ": " + it.second).c_str());

    return true;
}

std::string Config::getProperty(std::string key) {
    if (this->conf.find(key) == this->conf.end())
        return NULL;

    return this->conf[key];
}
