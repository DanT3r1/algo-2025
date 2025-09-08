#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "detail/exceptions.hpp"
#include "files/directory.hpp"

namespace filesystem {

class Fs {
public:
    Fs();
    ~Fs();

    void ChangeDir(const std::string& path);  // NOLINT(readability-make-member-function-const)
    std::string PWD() const;

    void ListFiles(const std::string& path = ".") const;
    void MakeDir(const std::string& path, bool create_parents = false);

    void CreateFile(const std::string& path, bool overwrite = false);
    void WriteToFile(const std::string& path, bool overwrite, std::ostream& in);
    void ShowFileContent(const std::string& path) const;

    void RemoveFile(const std::string& path);
    void FindFile(const std::string& filename) const;

private:
    std::vector<std::string> Split(const std::string& s, const std::string& sep) const;
    Directory* Navigate(const std::string& path, bool create_parents);
    Directory* root_;
    Directory* current_;
};

}  // namespace filesystem
