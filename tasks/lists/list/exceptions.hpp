#pragma once

#include <exception>
#include <string>
#include <string_view>

class ListIsEmptyException : public std::exception {
public:
    ListIsEmptyException() noexcept : message_("list is empty") {
    }
    explicit ListIsEmptyException(const char* msg) : message_(msg) {
    }
    explicit ListIsEmptyException(std::string msg) : message_(std::move(msg)) {
    }
    explicit ListIsEmptyException(std::string_view msg) : message_(msg) {
    }

    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};
