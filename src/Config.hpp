#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <map>
#include <string>

class Config {

    public:
        Config();

        bool init();
        std::string getProperty(std::string key);

    private:
        char* conf_file;
        std::map<std::string, std::string> conf;

};

#endif
