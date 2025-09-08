#include "file.hpp"

namespace filesystem {

File::File() : content_() {
}

void File::Write(const std::string& data, bool overwrite) {
    if (overwrite) {
        content_ = data;
    } else {
        content_ += data;
    }
}

const std::string& File::GetContent() const {
    return content_;
}

}  // namespace filesystem
