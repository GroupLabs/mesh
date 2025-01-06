#ifndef MYLOGGER_H
#define MYLOGGER_H

#include <iostream>
#include <string>

namespace Log {

// Enum for verbosity levels
enum VerbosityLevel { NONE, PEER_ERROR, PEER_WARN, PEER_INFO, PEER_DEBUG, PEER_DEBUG2 };

// External declarations for global variables
extern VerbosityLevel currentVerbosity;

extern const std::string RESET_COLOR;
extern const std::string RED_COLOR;
extern const std::string YELLOW_COLOR;
extern const std::string PURPLE_COLOR;
extern const std::string BLUE_COLOR;

// Function declaration
void logMessage(VerbosityLevel level, const std::string &message);

} // namespace MyLogger

#endif // MYLOGGER_H
