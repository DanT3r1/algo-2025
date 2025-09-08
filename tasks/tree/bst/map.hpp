#pragma once
#include <exception>
#include <functional>
#include <initializer_list>
#include <utility>
#include <vector>

struct KeyAbsent : public std::exception {
    const char* m;
    explicit KeyAbsent(const char* what) : m(what) {
    }
    const char* what() const noexcept override {
        return m;
    }
};

template <class Key, class Value, class Compare = std::less<Key>>
class Map {
public:
    Map() : root_(nullptr), n_(0), less_(Compare{}) {
    }

    Value& operator[](const Key& key) {
        Node* cur = root_;
        Node* parent = nullptr;
        while (cur != nullptr) {
            parent = cur;
            if (less_(key, cur->key)) {
                cur = cur->left;
            } else if (less_(cur->key, key)) {
                cur = cur->right;
            } else {
                return cur->val;
            }
        }
        Node* ins = new Node(key, Value{}, parent);
        LinkChild(parent, ins);
        ++n_;
        return ins->val;
    }

    void Insert(const std::pair<const Key, Value>& p) {
        InsertOrAssign(p.first, p.second);
    }

    void Insert(std::initializer_list<std::pair<const Key, Value>> items) {
        for (const auto& p : items) {
            InsertOrAssign(p.first, p.second);
        }
    }

    void Erase(const Key& key) {
        Node* z = FindNode(key);
        if (z == nullptr) {
            // без аллокаций, поведение соответствует тестам
            throw KeyAbsent("key not found");
        }
        EraseNode(z);
        --n_;
    }

    bool Find(const Key& key) const {
        return FindNode(key) != nullptr;
    }

    std::vector<std::pair<const Key, Value>> Values(bool is_increase = true) const noexcept {
        std::vector<std::pair<const Key, Value>> out;
        out.reserve(n_);
        InorderIterCollect(is_increase, out);
        return out;
    }

    void Clear() noexcept {
        DeletePostorderIter();
        root_ = nullptr;
        n_ = 0;
    }

    void Swap(Map& other) noexcept {
        using std::swap;
        swap(root_, other.root_);
        swap(n_, other.n_);
        swap(less_, other.less_);
    }

    bool IsEmpty() const noexcept {
        return n_ == 0;
    }
    size_t Size() const noexcept {
        return n_;
    }

    ~Map() {
        Clear();
    }

private:
    struct Node {
        Key key;
        Value val;
        Node* left;
        Node* right;
        Node* parent;
        Node(const Key& k, const Value& v, Node* p = nullptr)
            : key(k), val(v), left(nullptr), right(nullptr), parent(p) {
        }
    };

    Node* root_;
    size_t n_;
    Compare less_;

    // --- Внутренние утилиты ---

    Node* FindNode(const Key& k) const {
        Node* cur = root_;
        while (cur != nullptr) {
            if (less_(k, cur->key)) {
                cur = cur->left;
            } else if (less_(cur->key, k)) {
                cur = cur->right;
            } else {
                return cur;
            }
        }
        return nullptr;
    }

    void LinkChild(Node* parent, Node* child) {
        if (parent == nullptr) {
            root_ = child;
        } else if (less_(child->key, parent->key)) {
            parent->left = child;
            child->parent = parent;
        } else {
            parent->right = child;
            child->parent = parent;
        }
    }

    void InsertOrAssign(const Key& k, const Value& v) {
        Node* cur = root_;
        Node* parent = nullptr;
        while (cur != nullptr) {
            parent = cur;
            if (less_(k, cur->key)) {
                cur = cur->left;
            } else if (less_(cur->key, k)) {
                cur = cur->right;
            } else {
                cur->val = v;  // перезапись
                return;
            }
        }
        LinkChild(parent, new Node(k, v, parent));
        ++n_;
    }

    static Node* Min(Node* x) {
        Node* cur = x;
        while (cur != nullptr && cur->left != nullptr) {
            cur = cur->left;
        }
        return cur;
    }

    void ReplaceInParent(Node* u, Node* v) {
        Node* p = u->parent;
        if (p == nullptr) {
            root_ = v;
        } else if (p->left == u) {
            p->left = v;
        } else {
            p->right = v;
        }
        if (v != nullptr) {
            v->parent = p;
        }
    }

    void EraseNode(Node* z) {
        if (z->left == nullptr && z->right == nullptr) {
            ReplaceInParent(z, nullptr);
            delete z;
            return;
        }
        if (z->left == nullptr || z->right == nullptr) {
            Node* child = (z->left != nullptr) ? z->left : z->right;
            ReplaceInParent(z, child);
            delete z;
            return;
        }
        // два ребёнка: берём inorder-successor (минимум в правом поддереве)
        Node* s = Min(z->right);
        // перенесём данные преемника в z
        z->key = s->key;
        z->val = s->val;
        // удалим преемника (у него нет левого ребёнка)
        if (s->right != nullptr) {
            ReplaceInParent(s, s->right);
        } else {
            ReplaceInParent(s, nullptr);
        }
        delete s;
    }

    void InorderIterCollect(bool inc, std::vector<std::pair<const Key, Value>>& out) const {
        std::vector<Node*> st;
        Node* cur = root_;
        auto go_left = [&](Node* x) -> Node* { return inc ? x->left : x->right; };
        auto go_right = [&](Node* x) -> Node* { return inc ? x->right : x->left; };

        while (cur != nullptr || !st.empty()) {
            while (cur != nullptr) {
                st.push_back(cur);
                cur = go_left(cur);
            }
            Node* v = st.back();
            st.pop_back();
            out.emplace_back(v->key, v->val);
            cur = go_right(v);
        }
    }

    void DeletePostorderIter() noexcept {
        if (root_ == nullptr) {
            return;
        }
        std::vector<Node*> st;
        Node* cur = root_;
        Node* last = nullptr;

        while (!st.empty() || cur != nullptr) {
            if (cur != nullptr) {
                st.push_back(cur);
                cur = cur->left;
            } else {
                Node* peek = st.back();
                if (peek->right != nullptr && last != peek->right) {
                    cur = peek->right;
                } else {
                    st.pop_back();
                    last = peek;
                    delete peek;
                }
            }
        }
    }
};

// NOLINTBEGIN(readability-identifier-naming)
namespace std {
template <class K, class V, class C>
inline void swap(Map<K, V, C>& a, Map<K, V, C>& b) noexcept {
    a.Swap(b);
}
}  // namespace std
// NOLINTEND(readability-identifier-naming)
