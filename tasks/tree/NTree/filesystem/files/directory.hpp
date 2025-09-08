#pragma once

#include <string>

#include "../map/map.hpp"
#include "file.hpp"

namespace filesystem {

class Directory {
public:
    std::string name_;
    Directory* parent_;
    Map<std::string, Directory*> childs_;
    Map<std::string, File> files_;

    Directory(const std::string& name, Directory* parent);
    ~Directory();
};

}  // namespace filesystem
