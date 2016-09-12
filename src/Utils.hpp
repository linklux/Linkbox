#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <unistd.h>
#include <cstdlib>

class Utils {

    public:
        static const int P_READ = R_OK;
        static const int P_WRITE = W_OK;
        static const int P_EXECUTE = X_OK;

        static void terminate(int code = 1) { exit(code); };

        static void debug(const char * msg);
        static void error(const char * msg, bool terminate = false);
        static void printl(const char * msg);

        static bool exists(const char * path);
        static bool hasPermissions(const char * path, int permissions);

        static bool getDebugMode() { return debug_mode; };
        static void setDebugMode(bool mode) { debug_mode = mode; };

        static long getFileSize(const char * file);

    private:
        static bool debug_mode;

};

#endif
