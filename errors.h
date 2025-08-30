#ifndef ERRORS_H
#define ERRORS_H

#include <stdexcept>
#include <string>

enum class GameError {
    WINDOW_INIT_FAILED,
    RESOURCE_LOAD_FAILED,
    SYSTEM_INIT_FAILED
};

class GameException : public std::runtime_error {
public:
    GameException(GameError error, const std::string& details)
        : std::runtime_error(details), errorCode_(error) {}
    GameError getErrorCode() const { return errorCode_; }
private:
    GameError errorCode_;
};

#endif