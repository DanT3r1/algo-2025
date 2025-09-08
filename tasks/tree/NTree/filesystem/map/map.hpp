#pragma once

#include <functional>
#include <utility>
#include <vector>

namespace filesystem {

template <class Key, class Value, class Compare = std::less<Key>>
class Map {
private:
    struct Node {
        std::pair<Key, Value> kv;
        Node* left;
        Node* right;
        Node* parent;
        Node(const Key& k, const Value& v, Node* p = nullptr) : kv(k, v), left(nullptr), right(nullptr), parent(p) {
        }
    };

public:
    Map() : root_(nullptr), sz_(0), comp_() {
    }
    ~Map() {
        Clear();
    }

    // вставка или перезапись
    void Insert(const std::pair<const Key, Value>& p) {
        if (!root_) {
            root_ = new Node(p.first, p.second);
            ++sz_;
            return;
        }
        Node* cur = root_;
        Node* par = nullptr;
        while (cur) {
            par = cur;
            if (comp_(p.first, cur->kv.first)) {
                cur = cur->left;
            } else if (comp_(cur->kv.first, p.first)) {
                cur = cur->right;
            } else {
                cur->kv.second = p.second;
                return;
            }
        }
        Node* nn = new Node(p.first, p.second, par);
        if (comp_(p.first, par->kv.first)) {
            par->left = nn;
        } else {
            par->right = nn;
        }
        ++sz_;
    }

    // доступ/создание по ключу
    Value& operator[](const Key& key) {
        Node* cur = root_;
        Node* par = nullptr;
        while (cur) {
            par = cur;
            if (comp_(key, cur->kv.first)) {
                cur = cur->left;
            } else if (comp_(cur->kv.first, key)) {
                cur = cur->right;
            } else {
                return cur->kv.second;
            }
        }
        Node* nn = new Node(key, Value{}, par);
        if (!par) {
            root_ = nn;
        } else if (comp_(key, par->kv.first)) {
            par->left = nn;
        } else {
            par->right = nn;
        }
        ++sz_;
        return nn->kv.second;
    }

    bool Find(const Key& key) const noexcept {
        return FindNode(key) != nullptr;
    }

    void Erase(const Key& key) {
        Node* z = FindNode(key);
        if (!z) {
            return;  // в наших сценариях удаление по наличию проверяют заранее
        }
        if (!z->left || !z->right) {
            Node* c = z->left ? z->left : z->right;
            if (!z->parent) {
                root_ = c;
                if (c) {
                    c->parent = nullptr;
                }
            } else {
                if (z->parent->left == z) {
                    z->parent->left = c;
                } else {
                    z->parent->right = c;
                }
                if (c) {
                    c->parent = z->parent;
                }
            }
            delete z;
            --sz_;
            return;
        }
        // два потомка: заменить на следующий по возрастанию
        Node* s = z->right;
        while (s->left) {
            s = s->left;
        }
        z->kv.first = s->kv.first;
        z->kv.second = s->kv.second;
        // удалить successor (у него нет левого сына)
        if (s->parent->left == s) {
            s->parent->left = s->right;
        } else {
            s->parent->right = s->right;
        }
        if (s->right) {
            s->right->parent = s->parent;
        }
        delete s;
        --sz_;
    }

    std::vector<std::pair<const Key, Value>> Values(bool inc = true) const noexcept {
        std::vector<std::pair<const Key, Value>> out;
        out.reserve(sz_);
        if (inc) {
            Inorder(root_, out);
        } else {
            ReverseInorder(root_, out);
        }
        return out;
    }

    void Clear() noexcept {
        DeleteSubtree(root_);
        root_ = nullptr;
        sz_ = 0;
    }

    bool IsEmpty() const noexcept {
        return sz_ == 0;
    }
    size_t Size() const noexcept {
        return sz_;
    }

private:
    Node* root_;
    size_t sz_;
    Compare comp_;

    Node* FindNode(const Key& key) const {
        Node* cur = root_;
        while (cur) {
            if (comp_(key, cur->kv.first)) {
                cur = cur->left;
            } else if (comp_(cur->kv.first, key)) {
                cur = cur->right;
            } else {
                return cur;
            }
        }
        return nullptr;
    }

    static void DeleteSubtree(Node* n) {
        if (!n) {
            return;
        }
        DeleteSubtree(n->left);
        DeleteSubtree(n->right);
        delete n;
    }

    static void Inorder(Node* n, std::vector<std::pair<const Key, Value>>& v) {
        if (!n) {
            return;
        }
        Inorder(n->left, v);
        v.emplace_back(n->kv.first, n->kv.second);
        Inorder(n->right, v);
    }

    static void ReverseInorder(Node* n, std::vector<std::pair<const Key, Value>>& v) {
        if (!n) {
            return;
        }
        ReverseInorder(n->right, v);
        v.emplace_back(n->kv.first, n->kv.second);
        ReverseInorder(n->left, v);
    }
};

}  // namespace filesystem
