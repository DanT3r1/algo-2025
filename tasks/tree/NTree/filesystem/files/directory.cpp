#include "directory.hpp"

namespace filesystem {

Directory::Directory(const std::string& name, Directory* parent) : name_(name), parent_(parent) {
}

Directory::~Directory() {
    for (const auto& kv : childs_.Values(true)) {
        delete kv.second;
    }
}

}  // namespace filesystem
