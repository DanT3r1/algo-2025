#pragma once

#include <string>

namespace filesystem {

class File {
public:
    File();
    void Write(const std::string& data, bool overwrite);
    const std::string& GetContent() const;

private:
    std::string content_;
};

}  // namespace filesystem
