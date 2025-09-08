#include "fs.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>

namespace filesystem {

std::vector<std::string> Fs::Split(const std::string& s, const std::string& sep) const {
    std::vector<std::string> out;
    size_t pos = 0;
    while (true) {
        size_t nxt = s.find(sep, pos);
        if (nxt == std::string::npos) {
            break;
        }
        if (nxt > pos) {
            out.push_back(s.substr(pos, nxt - pos));
        }
        pos = nxt + sep.size();
    }
    if (pos < s.size()) {
        out.push_back(s.substr(pos));
    }
    return out;
}

Fs::Fs() : root_(new Directory("/", nullptr)), current_(root_) {
}

Fs::~Fs() {
    delete root_;
}

Directory* Fs::Navigate(const std::string& path, bool create_parents) {
    Directory* cur = (!path.empty() && path[0] == '/') ? root_ : current_;
    auto parts = Split(path, "/");
    for (const auto& token : parts) {
        if (token.empty() || token == ".") {
            continue;
        }
        if (token == "..") {
            if (!cur->parent_) {
                throw exceptions::FileNotFoundException("..");
            }
            cur = cur->parent_;
            continue;
        }
        if (!cur->childs_.Find(token)) {
            if (create_parents) {
                auto* d = new Directory(token, cur);
                cur->childs_.Insert({token, d});
                cur = d;
            } else {
                throw exceptions::FileNotFoundException(token);
            }
        } else {
            cur = cur->childs_[token];
        }
    }
    return cur;
}

std::string Fs::PWD() const {
    std::vector<std::string> names;
    for (auto* p = current_; p != nullptr; p = p->parent_) {
        names.push_back(p->name_);
    }
    std::reverse(names.begin(), names.end());
    std::ostringstream oss;
    for (size_t i = 0; i < names.size(); ++i) {
        oss << names[i];
        if (i + 1 < names.size() && names[i + 1] != "/") {
            oss << "/";
        }
    }
    return oss.str();
}

void Fs::ListFiles(const std::string& path) const {
    auto* self = const_cast<Fs*>(this);            // NOLINT(cppcoreguidelines-pro-type-const-cast)
    Directory* tgt = self->Navigate(path, false);  // NOLINT(cppcoreguidelines-pro-type-const-cast)
    for (const auto& kv : tgt->childs_.Values(true)) {
        std::cout << kv.first << "";
    }
    for (const auto& kv : tgt->files_.Values(true)) {
        std::cout << kv.first << "";
    }
}

void Fs::ChangeDir(const std::string& path) {  // NOLINT(readability-make-member-function-const)
    current_ = Navigate(path, false);
    std::cout << PWD() << "";
}

void Fs::MakeDir(const std::string& path, bool create_parents) {
    auto pos = path.find_last_of('/');
    std::string dir = (pos == std::string::npos ? "" : path.substr(0, pos));
    std::string name = (pos == std::string::npos ? path : path.substr(pos + 1));
    Directory* parent = Navigate(dir, create_parents);
    if (!parent->childs_.Find(name)) {
        auto* d = new Directory(name, parent);
        parent->childs_.Insert({name, d});
    }
}

void Fs::CreateFile(const std::string& path, bool overwrite) {
    auto pos = path.find_last_of('/');
    std::string dir = (pos == std::string::npos ? "" : path.substr(0, pos));
    std::string name = (pos == std::string::npos ? path : path.substr(pos + 1));
    Directory* tgt = Navigate(dir, false);
    if (tgt->files_.Find(name)) {
        if (!overwrite) {
            throw exceptions::FileExistsException(name);
        }
        tgt->files_[name] = File();
    } else {
        tgt->files_.Insert({name, File()});
    }
}

void Fs::WriteToFile(const std::string& path, bool overwrite, std::ostream& in) {
    auto pos = path.find_last_of('/');
    std::string dir = (pos == std::string::npos ? "" : path.substr(0, pos));
    std::string name = (pos == std::string::npos ? path : path.substr(pos + 1));
    Directory* tgt = Navigate(dir, false);
    if (!tgt->files_.Find(name)) {
        throw exceptions::FileNotFoundException(name);
    }
    std::ostringstream buf;
    buf << in.rdbuf();
    tgt->files_[name].Write(buf.str(), overwrite);
}

void Fs::ShowFileContent(const std::string& path) const {
    auto pos = path.find_last_of('/');
    std::string dir = (pos == std::string::npos ? "" : path.substr(0, pos));
    std::string name = (pos == std::string::npos ? path : path.substr(pos + 1));
    auto* self = const_cast<Fs*>(this);           // NOLINT(cppcoreguidelines-pro-type-const-cast)
    Directory* tgt = self->Navigate(dir, false);  // NOLINT(cppcoreguidelines-pro-type-const-cast)
    if (!tgt->files_.Find(name)) {
        throw exceptions::FileNotFoundException(name);
    }
    std::cout << tgt->files_[name].GetContent();
}

void Fs::RemoveFile(const std::string& path) {
    if (path == "/" || path.empty()) {
        for (const auto& kv : root_->childs_.Values(true)) {
            delete kv.second;
        }
        root_->childs_.Clear();
        root_->files_.Clear();
        current_ = root_;
        return;
    }
    // пробуем как каталог
    try {
        Directory* d = Navigate(path, false);
        if (d->parent_) {
            d->parent_->childs_.Erase(d->name_);
            delete d;
            return;
        }
        throw exceptions::FileNotFoundException(path);
    } catch (...) {
        // попробуем как файл
    }
    auto pos = path.find_last_of('/');
    std::string dir = (pos == std::string::npos ? "" : path.substr(0, pos));
    std::string name = (pos == std::string::npos ? path : path.substr(pos + 1));
    Directory* tgt = Navigate(dir, false);
    if (!tgt->files_.Find(name)) {
        throw exceptions::FileNotFoundException(name);
    }
    tgt->files_.Erase(name);
}

void Fs::FindFile(const std::string& filename) const {
    bool found = false;
    std::function<void(Directory*, const std::string&, const std::string&)> rec;
    rec = [&](Directory* d, const std::string& nm, const std::string& pref) {
        std::string here = pref;
        if (d->parent_) {
            if (!here.empty() && here.back() != '/') {
                here += "/";
            }
            here += d->name_;
        }
        for (const auto& kv : d->files_.Values(true)) {
            if (kv.first == nm) {
                std::cout << (here.empty() ? "/" : here + "/") << kv.first << "";
                found = true;
            }
        }
        for (const auto& kv : d->childs_.Values(true)) {
            rec(kv.second, nm, here);
        }
    };
    rec(root_, filename, "");
    if (!found) {
        throw exceptions::FileNotFoundException(filename);
    }
}

}  // namespace filesystem
