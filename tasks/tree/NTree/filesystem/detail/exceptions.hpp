#pragma once

#include <exception>
#include <string>

namespace filesystem::exceptions {

struct FileNotFoundException : std::exception {
    explicit FileNotFoundException(const std::string& what_arg) : msg_("FileNotFound: " + what_arg) {
    }
    const char* what() const noexcept override {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

struct FileExistsException : std::exception {
    explicit FileExistsException(const std::string& what_arg) : msg_("FileExists: " + what_arg) {
    }
    const char* what() const noexcept override {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

struct DirectoryNotFoundException : std::exception {
    explicit DirectoryNotFoundException(const std::string& what_arg) : msg_("DirectoryNotFound: " + what_arg) {
    }
    const char* what() const noexcept override {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

}  // namespace filesystem::exceptions
