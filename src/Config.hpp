#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <map>
#include <string>

class Config {

    public:
        Config(const char * file);

        bool init();
        std::string getProperty(std::string key);

    private:
        std::string conf_file;
        std::map<std::string, std::string> conf;

};

#endif
