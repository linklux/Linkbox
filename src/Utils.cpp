#include "Utils.hpp"

#include <iostream>

bool Utils::debug_mode = false;

/**
 * Prints an info message if debug mode is enabled.
 */
void Utils::debug(const char * msg) {
    if (!debug_mode)
        return;

    std::cout << "[INFO] " << msg << std::endl;
}

/**
 * Prints an error message. Support for program termination.
 */
void Utils::error(const char * msg, bool terminate) {
    std::cout << "[ERR] " << msg << std::endl;

    if (terminate) {
        std::cout << "Terminated!" << std::endl;
        exit(1);
    }
}

/**
 * Prints given message with a line-break
 */
void Utils::printl(const char * msg) {
    std::cout << msg << std::endl;
}

/**
 * Checks whether a file or folder exists.
 */
bool Utils::exists(const char * path) {
    return access(path, F_OK) == 0;
}

/**
 * Checks a file or folder for requested permissions. First makes sure the file
 * or folder exists. Multiple permissions can be given by separating them by the
 * binary OR operator.
 *
 * Available options are P_READ, P_WRITE and P_EXECUTE
 */
bool Utils::hasPermissions(const char * path, int permissions) {
    if (!Utils::exists(path))
        return false;

    return access(path, permissions) == 0;
}
