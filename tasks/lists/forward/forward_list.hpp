#pragma once

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <utility>

#include "exceptions.hpp"

// NOLINTNEXTLINE(readability-identifier-naming)
template <typename T>
class ForwardList {
private:
    struct Node {
        T value;
        Node* next = nullptr;
        Node() = default;
        explicit Node(const T& v) : value(v) {
        }
    };

    Node* head_ = nullptr;
    size_t size_ = 0;

private:
    static void LinkAfter(Node* pos, Node* node) noexcept {
        node->next = pos->next;
        pos->next = node;
    }

    static Node* UnlinkAfter(Node* pos) noexcept {
        Node* victim = pos->next;
        pos->next = victim->next;
        delete victim;
        return pos->next;
    }

    void CopyFrom(const ForwardList& other) {
        if (!other.head_) {
            return;
        }
        Node* src = other.head_;
        Node* dst_head = new Node(src->value);
        Node* dst_tail = dst_head;
        size_ = 1;
        src = src->next;
        try {
            while (src) {
                dst_tail->next = new Node(src->value);
                dst_tail = dst_tail->next;
                ++size_;
                src = src->next;
            }
        } catch (...) {
            // очистка partially built
            Node* cur = dst_head;
            while (cur) {
                Node* nxt = cur->next;
                delete cur;
                cur = nxt;
            }
            size_ = 0;
            throw;
        }
        head_ = dst_head;
    }

public:
    // ===== ctors / dtors =====
    ForwardList() = default;

    explicit ForwardList(size_t count) {
        if (count == 0) {
            return;
        }
        // Строим "хвостом", чтобы не переворачивать порядок.
        head_ = new Node(T{});
        Node* tail = head_;
        size_ = 1;
        for (size_t i = 1; i < count; ++i) {
            tail->next = new Node(T{});
            tail = tail->next;
            ++size_;
        }
    }

    ForwardList(std::initializer_list<T> init) {
        if (init.size() == 0) {
            return;
        }
        auto it = init.begin();
        head_ = new Node(*it++);
        Node* tail = head_;
        size_ = 1;
        for (; it != init.end(); ++it) {
            tail->next = new Node(*it);
            tail = tail->next;
            ++size_;
        }
    }

    ForwardList(const ForwardList& other) {
        CopyFrom(other);
    }

    ForwardList& operator=(const ForwardList& other) {
        if (this == &other) {
            return *this;
        }
        ForwardList tmp(other);
        Swap(tmp);
        return *this;
    }

    ~ForwardList() {
        Clear();
    }

    // ===== observers =====
    bool IsEmpty() const {
        return size_ == 0;
    }
    size_t Size() const {
        return size_ == 0 ? 0 : size_;
    }

    T& Front() {
        if (!head_) {
            throw ListIsEmptyException();
        }
        return head_->value;
    }
    const T& Front() const {
        if (!head_) {
            throw ListIsEmptyException();
        }
        return head_->value;
    }

    // ===== modifiers =====
    void PushFront(const T& value) {
        Node* n = new Node(value);
        n->next = head_;
        head_ = n;
        ++size_;
    }

    void PopFront() {
        if (!head_) {
            throw ListIsEmptyException();
        }
        Node* v = head_;
        head_ = head_->next;
        delete v;
        --size_;
    }

    void Clear() {
        Node* cur = head_;
        while (cur) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        head_ = nullptr;
        size_ = 0;
    }

    void Swap(ForwardList& other) {
        std::swap(head_, other.head_);
        std::swap(size_, other.size_);
    }

    // ===== iterators =====
    class Iterator {
    public:
        // NOLINTBEGIN(readability-identifier-naming)
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        // NOLINTEND(readability-identifier-naming)

        Iterator() : node_(nullptr) {
        }
        explicit Iterator(Node* n) : node_(n) {
        }

        reference operator*() const {
            return node_->value;
        }
        pointer operator->() const {
            return &node_->value;
        }

        Iterator& operator++() {
            node_ = node_->next;
            return *this;
        }
        Iterator operator++(int) {
            Iterator t = *this;
            ++(*this);
            return t;
        }

        bool operator==(const Iterator& rhs) const {
            return node_ == rhs.node_;
        }
        bool operator!=(const Iterator& rhs) const {
            return node_ != rhs.node_;
        }

        Node* GetNode() const {
            return node_;
        }

    private:
        Node* node_;
        friend class ForwardList;
    };

    class ConstIterator {
    public:
        // NOLINTBEGIN(readability-identifier-naming)
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;
        // NOLINTEND(readability-identifier-naming)

        ConstIterator() : node_(nullptr) {
        }
        explicit ConstIterator(const Node* n) : node_(n) {
        }

        reference operator*() const {
            return node_->value;
        }
        pointer operator->() const {
            return &node_->value;
        }

        ConstIterator& operator++() {
            node_ = node_->next;
            return *this;
        }
        ConstIterator operator++(int) {
            ConstIterator t = *this;
            ++(*this);
            return t;
        }

        bool operator==(const ConstIterator& rhs) const {
            return node_ == rhs.node_;
        }
        bool operator!=(const ConstIterator& rhs) const {
            return node_ != rhs.node_;
        }

    private:
        const Node* node_;
    };

    Iterator Begin() {
        return Iterator(head_);
    }
    Iterator End() {
        return Iterator(nullptr);
    }

    ConstIterator Begin() const {
        return ConstIterator(head_);
    }
    ConstIterator End() const {
        return ConstIterator(nullptr);
    }

    // ===== search / insert-after / erase-after =====
    Iterator Find(const T& value) {
        for (Node* cur = head_; cur; cur = cur->next) {
            if (cur->value == value) {
                return Iterator(cur);
            }
        }
        return End();
    }

    Iterator InsertAfter(Iterator pos, const T& value) {
        Node* base = pos.GetNode();  // предполагается валидным
        Node* n = new Node(value);
        LinkAfter(base, n);
        ++size_;
        return Iterator(n);
    }

    Iterator EraseAfter(Iterator pos) {
        Node* base = pos.GetNode();  // предполагается валидным
        Node* target = base->next;
        if (!target) {
            return End();
        }
        Node* next = UnlinkAfter(base);
        --size_;
        return Iterator(next);
    }
};

// NOLINTBEGIN(readability-identifier-naming)
namespace std {
template <typename T>
inline void swap(ForwardList<T>& a, ForwardList<T>& b) {
    a.Swap(b);
}
}  // namespace std
// NOLINTEND(readability-identifier-naming)
