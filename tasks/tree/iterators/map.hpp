#pragma once

#include <functional>
#include <iterator>
#include <utility>
#include <vector>

template <class Key, class Value, class Compare = std::less<Key>>
class Map {
private:
    struct Node {
        std::pair<const Key, Value> kv;
        Node* left;
        Node* right;
        Node(const Key& k, const Value& v) : kv(k, v), left(nullptr), right(nullptr) {
        }
    };

    struct NotFound final {};

    Node* root_ = nullptr;
    size_t sz_ = 0;
    Compare comp_{};

    Node** LinkTo(const Key& key) {
        Node** link = &root_;
        while (*link) {
            if (comp_(key, (*link)->kv.first)) {
                link = &((*link)->left);
            } else if (comp_((*link)->kv.first, key)) {
                link = &((*link)->right);
            } else {
                break;
            }
        }
        return link;
    }

    static void DeleteSubtree(Node* n) {
        if (!n) {
            return;
        }
        DeleteSubtree(n->left);
        DeleteSubtree(n->right);
        delete n;
    }

public:
    class Iterator {
    public:
        using ValueType = std::pair<const Key, Value>;
        using Reference = ValueType&;
        using Pointer = ValueType*;
        using DifferenceType = std::ptrdiff_t;
        using IteratorCategory = std::forward_iterator_tag;

        Iterator() = default;

        Reference operator*() const {
            return cur_->kv;
        }
        Pointer operator->() const {
            return &cur_->kv;
        }

        bool operator==(const Iterator& o) const noexcept {
            return cur_ == o.cur_;
        }
        bool operator!=(const Iterator& o) const noexcept {
            return cur_ != o.cur_;
        }

        Iterator& operator++() {
            if (!cur_) {
                return *this;
            }
            if (cur_->right) {
                cur_ = cur_->right;
                while (cur_->left) {
                    st_.push_back(cur_);
                    cur_ = cur_->left;
                }
                return *this;
            }
            while (!st_.empty() && st_.back()->right == cur_) {
                cur_ = st_.back();
                st_.pop_back();
            }
            if (st_.empty()) {
                cur_ = nullptr;
            } else {
                cur_ = st_.back();
                st_.pop_back();
            }
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

    private:
        friend class Map<Key, Value, Compare>;

        explicit Iterator(Node* root) {
            if (!root) {
                cur_ = nullptr;
                return;
            }
            Node* cur = root;
            while (cur->left) {
                st_.push_back(cur);
                cur = cur->left;
            }
            cur_ = cur;
        }

        static Iterator FromTarget(Node* root, Node* target) {
            Iterator it;
            if (!target) {
                return it;
            }
            Node* cur = root;
            while (cur && cur != target) {
                it.st_.push_back(cur);
                if (Compare{}(target->kv.first, cur->kv.first)) {
                    cur = cur->left;
                } else {
                    cur = cur->right;
                }
            }
            it.cur_ = target;
            return it;
        }

        Node* cur_ = nullptr;
        std::vector<Node*> st_;
    };

    Map() = default;
    ~Map() {
        Clear();
    }

    Iterator Begin() const noexcept {
        return Iterator(root_);
    }
    Iterator End() const noexcept {
        return Iterator();
    }

    Value& operator[](const Key& key) {
        Node** link = LinkTo(key);
        if (*link) {
            return (*link)->kv.second;
        }
        *link = new Node(key, Value{});
        ++sz_;
        return (*link)->kv.second;
    }

    void Insert(const std::pair<const Key, Value>& p) {
        Node** link = LinkTo(p.first);
        if (*link) {
            (*link)->kv.second = p.second;
            return;
        }
        *link = new Node(p.first, p.second);
        ++sz_;
    }

    void Insert(std::initializer_list<std::pair<const Key, Value>> list) {
        for (const auto& x : list) {
            Insert(x);
        }
    }

    void Erase(const Key& key) {
        Node** link = LinkTo(key);
        Node* z = *link;
        if (!z) {
            throw NotFound{};
        }

        if (!z->left) {
            *link = z->right;
            delete z;
        } else if (!z->right) {
            *link = z->left;
            delete z;
        } else {
            Node** succ = &(z->right);
            while ((*succ)->left) {
                succ = &((*succ)->left);
            }
            Node* s = *succ;
            *succ = s->right;
            s->left = z->left;
            s->right = z->right;
            *link = s;
            delete z;
        }
        --sz_;
    }

    Iterator Find(const Key& key) const {
        Node* cur = root_;
        while (cur) {
            if (comp_(key, cur->kv.first)) {
                cur = cur->left;
            } else if (comp_(cur->kv.first, key)) {
                cur = cur->right;
            } else {
                return Iterator::FromTarget(root_, cur);
            }
        }
        return End();
    }

    std::vector<std::pair<const Key, Value>> Values(bool inc = true) const noexcept {
        std::vector<std::pair<const Key, Value>> out;
        out.reserve(sz_);
        if (inc) {
            std::vector<Node*> st;
            Node* cur = root_;
            while (cur || !st.empty()) {
                while (cur) {
                    st.push_back(cur);
                    cur = cur->left;
                }
                cur = st.back();
                st.pop_back();
                out.push_back(cur->kv);
                cur = cur->right;
            }
        } else {
            std::vector<Node*> st;
            Node* cur = root_;
            while (cur || !st.empty()) {
                while (cur) {
                    st.push_back(cur);
                    cur = cur->right;
                }
                cur = st.back();
                st.pop_back();
                out.push_back(cur->kv);
                cur = cur->left;
            }
        }
        return out;
    }

    void Clear() noexcept {
        DeleteSubtree(root_);
        root_ = nullptr;
        sz_ = 0;
    }

    void Swap(Map& other) noexcept {
        using std::swap;
        swap(root_, other.root_);
        swap(sz_, other.sz_);
        swap(comp_, other.comp_);
    }

    bool IsEmpty() const noexcept {
        return sz_ == 0;
    }
    size_t Size() const noexcept {
        return sz_;
    }
};

// перегрузка std::swap для нашего типа
namespace std {
template <class Key, class Value, class Compare>
inline void swap(Map<Key, Value, Compare>& a, Map<Key, Value, Compare>& b) noexcept {  // NOLINT
    a.Swap(b);
}
}  // namespace std
