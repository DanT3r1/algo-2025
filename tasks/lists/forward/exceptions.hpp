#pragma once

#include <exception>
#include <string>
#include <string_view>

class ListIsEmptyException : public std::exception {
public:
    ListIsEmptyException() noexcept : msg_("List is empty") {
    }
    explicit ListIsEmptyException(const char* m) : msg_(m) {
    }
    explicit ListIsEmptyException(std::string m) : msg_(std::move(m)) {
    }
    explicit ListIsEmptyException(std::string_view m) : msg_(m) {
    }

    const char* what() const noexcept override {
        return msg_.c_str();
    }

private:
    std::string msg_;
};
