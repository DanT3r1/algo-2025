#pragma once

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <utility>

#include "exceptions.hpp"

// NOLINTNEXTLINE(readability-identifier-naming)
template <typename T>
class List {
private:
    struct Node {
        T value;
        Node* prev = nullptr;
        Node* next = nullptr;

        explicit Node(const T& v) : value(v) {
        }
        Node() = default;  // для sentinel
    };

    Node* sentinel_ = nullptr;  // круговой список: sentinel_->next — голова, sentinel_->prev — хвост
    size_t size_ = 0;

private:
    // Создать пустой круговой список с sentinel-узлом
    void InitEmpty() {
        sentinel_ = new Node();
        sentinel_->next = sentinel_;
        sentinel_->prev = sentinel_;
        size_ = 0;
    }

    // Разорвать все узлы (кроме sentinel) и сбросить размер
    void DestroyAll() noexcept {
        Node* cur = sentinel_->next;
        while (cur != sentinel_) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        sentinel_->next = sentinel_;
        sentinel_->prev = sentinel_;
        size_ = 0;
    }

    // Вставить уже созданный node ПЕРЕД узлом pos (pos может быть sentinel)
    void LinkBefore(Node* pos, Node* node) noexcept {
        Node* p = pos->prev;
        node->prev = p;
        node->next = pos;
        p->next = node;
        pos->prev = node;
        ++size_;
    }

    // Удалить узел (не sentinel)
    void UnlinkAndDelete(Node* node) noexcept {
        Node* p = node->prev;
        Node* n = node->next;
        p->next = n;
        n->prev = p;
        delete node;
        --size_;
    }

    void CopyFrom(const List& other) {
        for (Node* cur = other.sentinel_->next; cur != other.sentinel_; cur = cur->next) {
            PushBack(cur->value);
        }
    }

public:
    // ===== ctors / dtors =====
    List() {
        InitEmpty();
    }

    explicit List(size_t count) {
        InitEmpty();
        for (size_t i = 0; i < count; ++i) {
            PushBack(T());
        }
    }

    List(std::initializer_list<T> init) {
        InitEmpty();
        for (const auto& v : init) {
            PushBack(v);
        }
    }

    List(const List& other) {
        InitEmpty();
        CopyFrom(other);
    }

    List& operator=(const List& other) {
        if (this != &other) {
            Clear();
            CopyFrom(other);
        }
        return *this;
    }

    ~List() {
        DestroyAll();
        delete sentinel_;
        sentinel_ = nullptr;
    }

    // ===== modifiers (basic) =====
    void PushBack(const T& v) {
        // вставка перед sentinel (в конец)
        LinkBefore(sentinel_, new Node(v));
    }

    void PushFront(const T& v) {
        // вставка перед текущей головой (sentinel_->next)
        LinkBefore(sentinel_->next, new Node(v));
    }

    void PopBack() {
        if (IsEmpty()) {
            throw ListIsEmptyException("PopBack on empty list");
        }
        UnlinkAndDelete(sentinel_->prev);
    }

    void PopFront() {
        if (IsEmpty()) {
            throw ListIsEmptyException("PopFront on empty list");
        }
        UnlinkAndDelete(sentinel_->next);
    }

    void Clear() {
        DestroyAll();
    }

    // ===== queries =====
    bool IsEmpty() const {
        return size_ == 0;
    }

    size_t Size() const {
        return size_;
    }

    T& Front() {
        if (IsEmpty()) {
            throw ListIsEmptyException("Front on empty list");
        }
        return sentinel_->next->value;
    }

    T& Back() {
        if (IsEmpty()) {
            throw ListIsEmptyException("Back on empty list");
        }
        return sentinel_->prev->value;
    }

    void Swap(List& other) {
        std::swap(sentinel_, other.sentinel_);
        std::swap(size_, other.size_);
    }

    // ===== iterator =====
    template <typename Ptr>
    class ListIter {
    public:
        // NOLINTBEGIN(readability-identifier-naming)
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        // NOLINTEND(readability-identifier-naming)

        ListIter(Ptr p, const List* owner) : cur_(p), parent_(owner) {
        }

        bool operator==(const ListIter& rhs) const {
            return cur_ == rhs.cur_;
        }
        bool operator!=(const ListIter& rhs) const {
            return !(*this == rhs);
        }

        reference operator*() const {
            return cur_->value;
        }
        pointer operator->() const {
            return &cur_->value;
        }

        // prefix ++
        ListIter& operator++() {
            // End() представлен как nullptr — инкремент от End() оставим как есть
            if (cur_ != nullptr) {
                cur_ = cur_->next;
                if (cur_ == parent_->sentinel_) {
                    cur_ = nullptr;  // перешли за хвост -> End()
                }
            }
            return *this;
        }

        // postfix ++
        ListIter operator++(int) {
            ListIter tmp = *this;
            ++(*this);
            return tmp;
        }

        // prefix --
        ListIter& operator--() {
            // с End() (--End) должен прийти к хвосту
            if (cur_ == nullptr) {
                // пустой список обработаем безопасно
                if (parent_->IsEmpty()) {
                    return *this;  // остаёмся End()
                }
                cur_ = parent_->sentinel_->prev;
                return *this;
            }
            // обычный шаг назад
            if (cur_->prev == parent_->sentinel_) {
                cur_ =
                    nullptr;  // ушли перед головой — это эквивалент End() с точки зрения теста (-- от Begin недоступен)
            } else {
                cur_ = cur_->prev;
            }
            return *this;
        }

        // postfix --
        ListIter operator--(int) {
            ListIter tmp = *this;
            --(*this);
            return tmp;
        }

    private:
        Ptr cur_;
        const List* parent_;
        friend class List;
    };

    using Iterator = ListIter<Node*>;

    Iterator Begin() const {
        if (IsEmpty()) {
            return End();
        }
        return Iterator(sentinel_->next, this);
    }

    Iterator End() const {
        return Iterator(nullptr, this);
    }

    // Поиск первого вхождения значения
    Iterator Find(const T& value) const {
        for (Node* cur = sentinel_->next; cur != sentinel_; cur = cur->next) {
            if (cur->value == value) {
                return Iterator(cur, this);
            }
        }
        return End();
    }

    void Erase(Iterator it) {
        if (IsEmpty() || it == End()) {
            throw ListIsEmptyException("Erase on empty or end");
        }
        UnlinkAndDelete(it.cur_);
    }

    void Insert(Iterator it, const T& value) {
        if (it == Begin()) {
            PushFront(value);
            return;
        }
        if (it == End()) {
            PushBack(value);
            return;
        }
        // Вставка перед текущим узлом итератора
        Node* pos = it.cur_;
        Node* node = new Node(value);
        LinkBefore(pos, node);
    }
};

// ADL/ std::swap поддержка
namespace std {
template <typename T>
// NOLINTNEXTLINE(readability-identifier-naming)
inline void swap(List<T>& a, List<T>& b) {
    a.Swap(b);
}
}  // namespace std
