#include "include/log.h"

namespace Log {

// Initialize global variables
VerbosityLevel currentVerbosity = PEER_DEBUG2;

const std::string RESET_COLOR = "\033[0m";
const std::string RED_COLOR = "\033[31m";
const std::string YELLOW_COLOR = "\033[33m";
const std::string PURPLE_COLOR = "\033[35m";
const std::string BLUE_COLOR = "\033[34m";

// Function definition
void logMessage(VerbosityLevel level, const std::string &message) {
    if (level <= currentVerbosity) {
        switch (level) {
            case PEER_ERROR:
                std::cerr << RED_COLOR << "ERROR: " << message << RESET_COLOR
                          << std::endl;
                break;
            case PEER_WARN:
                std::cerr << YELLOW_COLOR << "WARN: " << message << RESET_COLOR
                          << std::endl;
                break;
            case PEER_INFO:
                std::cout << "INFO: " << message << std::endl;
                break;
            case PEER_DEBUG:
                std::cout << BLUE_COLOR << "DEBUG: " << message << RESET_COLOR
                          << std::endl;
                break;
            case PEER_DEBUG2:
                std::cout << PURPLE_COLOR << "DEBUG2: " << message << RESET_COLOR
                          << std::endl;
                break;
            default:
                break;
        }
    }
}

} // namespace MyLogger
