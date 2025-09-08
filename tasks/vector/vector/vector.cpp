#include "vector.hpp"

#include <algorithm>    // std::min
#include <cstdlib>      // free
#include <new>          // ::operator new, placement new
#include <type_traits>  // std::is_pointer_v
#include <utility>      // std::move, std::swap

namespace {
constexpr size_t KInitialCapacity = 10;
}

// ===== helpers =====
template <typename T>
void Vector<T>::DestroyRange(size_t from, size_t to) noexcept {
    for (size_t i = from; i < to; ++i) {
        (data_ + i)->~T();
    }
}

// Специализация: Vector<void*> освобождает память, выделенную через malloc
template <>
inline void Vector<void*>::DestroyRange(size_t from, size_t to) noexcept {
    for (size_t i = from; i < to; ++i) {
        if (data_[i]) {
            free(data_[i]);
            data_[i] = nullptr;
        }
    }
}

template <typename T>
size_t Vector<T>::GrowCapacity(size_t cur, size_t need) {
    if (cur == 0) {
        cur = KInitialCapacity;  // первая аллокация — 10
    }
    while (cur < need) {
        cur *= 2;  // удвоение до нужного
    }
    return cur;
}

template <typename T>
void Vector<T>::ReallocateAndMove(size_t new_cap) {
    T* new_mem = static_cast<T*>(::operator new(sizeof(T) * new_cap));
    size_t i = 0;
    try {
        for (; i < size_; ++i) {
            new (new_mem + i) T(std::move(data_[i]));
            if constexpr (std::is_pointer_v<T>) {
                // чтобы DestroyRange не освободил уже "переехавший" указатель
                data_[i] = nullptr;
            }
        }
    } catch (...) {
        for (size_t j = 0; j < i; ++j) {
            (new_mem + j)->~T();
        }
        ::operator delete(new_mem);
        throw;
    }
    DestroyRange(0, size_);
    ::operator delete(data_);
    data_ = new_mem;
    capacity_ = new_cap;
}

template <typename T>
void Vector<T>::EnsureCapacityForInsert(size_t add) {
    if (size_ + add <= capacity_) {
        return;
    }
    ReallocateAndMove(GrowCapacity(capacity_, size_ + add));
}

// ===== ctors / dtors =====
template <typename T>
Vector<T>::Vector() = default;

template <typename T>
Vector<T>::Vector(size_t count, const T& value) {
    if (count == 0) {
        return;
    }
    capacity_ = GrowCapacity(0, count);
    data_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));
    size_ = 0;
    try {
        for (; size_ < count; ++size_) {
            new (data_ + size_) T(value);
        }
    } catch (...) {
        DestroyRange(0, size_);
        ::operator delete(data_);
        data_ = nullptr;
        size_ = capacity_ = 0;
        throw;
    }
}

template <typename T>
Vector<T>::Vector(const Vector& other) {
    if (other.size_ == 0) {
        return;
    }
    capacity_ = other.size_;
    data_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));
    size_ = 0;
    try {
        for (; size_ < other.size_; ++size_) {
            new (data_ + size_) T(other.data_[size_]);
        }
    } catch (...) {
        DestroyRange(0, size_);
        ::operator delete(data_);
        data_ = nullptr;
        size_ = capacity_ = 0;
        throw;
    }
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

template <typename T>
Vector<T>::Vector(std::initializer_list<T> init) {
    if (init.size() == 0) {
        return;
    }
    capacity_ = GrowCapacity(0, init.size());
    data_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));
    size_ = 0;
    try {
        for (const T& v : init) {
            new (data_ + size_) T(v);
            ++size_;
        }
    } catch (...) {
        DestroyRange(0, size_);
        ::operator delete(data_);
        data_ = nullptr;
        size_ = capacity_ = 0;
        throw;
    }
}

template <typename T>
Vector<T>::~Vector() {
    DestroyRange(0, size_);
    ::operator delete(data_);
}

// ===== assign / swap =====
template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& other) {
    if (this == &other) {
        return *this;
    }
    if (other.size_ == 0) {
        DestroyRange(0, size_);
        ::operator delete(data_);
        data_ = nullptr;
        size_ = capacity_ = 0;
        return *this;
    }
    if (other.size_ > capacity_) {
        Vector tmp(other);
        Swap(tmp);
        return *this;
    }
    size_t i = 0;
    for (; i < std::min(size_, other.size_); ++i) {
        data_[i] = other.data_[i];
    }
    for (; i < other.size_; ++i) {
        new (data_ + i) T(other.data_[i]);
    }
    DestroyRange(other.size_, size_);
    size_ = other.size_;
    return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    DestroyRange(0, size_);
    ::operator delete(data_);
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
    return *this;
}

template <typename T>
void Vector<T>::Swap(Vector& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
}

// ===== capacity =====
template <typename T>
void Vector<T>::Reserve(size_t new_cap) {
    if (new_cap <= capacity_) {
        return;
    }
    ReallocateAndMove(new_cap);
}

// Resize без значения
template <typename T>
void Vector<T>::Resize(size_t new_size) {
    if (new_size == size_) {
        return;
    }
    if (new_size < size_) {
        DestroyRange(new_size, size_);
        size_ = new_size;
        return;
    }
    EnsureCapacityForInsert(new_size - size_);
    size_t i = size_;
    try {
        for (; i < new_size; ++i) {
            new (data_ + i) T();
        }
    } catch (...) {
        DestroyRange(size_, i);
        throw;
    }
    size_ = new_size;
}

// Resize с заполнением значением (lvalue)
template <typename T>
void Vector<T>::Resize(size_t new_size, const T& value) {
    if (new_size <= size_) {
        Resize(new_size);
        return;
    }
    EnsureCapacityForInsert(new_size - size_);
    size_t i = size_;
    try {
        for (; i < new_size; ++i) {
            new (data_ + i) T(value);
        }
    } catch (...) {
        DestroyRange(size_, i);
        throw;
    }
    size_ = new_size;
}

// Resize с заполнением значением (rvalue)
template <typename T>
void Vector<T>::Resize(size_t new_size, T&& value) {
    if (new_size <= size_) {
        Resize(new_size);
        return;
    }
    EnsureCapacityForInsert(new_size - size_);
    size_t i = size_;
    try {
        for (; i < new_size; ++i) {
            new (data_ + i) T(value);  // заполняем одинаковым значением
        }
    } catch (...) {
        DestroyRange(size_, i);
        throw;
    }
    size_ = new_size;
}

template <typename T>
void Vector<T>::Clear() noexcept {
    DestroyRange(0, size_);
    size_ = 0;
}

// ===== modifiers =====
template <typename T>
void Vector<T>::PushBack(const T& value) {
    EnsureCapacityForInsert(1);
    new (data_ + size_) T(value);
    ++size_;
}

template <typename T>
void Vector<T>::PushBack(T&& value) {
    EnsureCapacityForInsert(1);
    new (data_ + size_) T(std::move(value));
    ++size_;
}

template <typename T>
template <class... Args>
T& Vector<T>::EmplaceBack(Args&&... args) {
    EnsureCapacityForInsert(1);
    new (data_ + size_) T(std::forward<Args>(args)...);
    ++size_;
    return Back();
}

template <typename T>
void Vector<T>::PopBack() noexcept {
    (data_ + (size_ - 1))->~T();
    --size_;
}

template <typename T>
T* Vector<T>::Insert(size_t pos, const T& value) {
    // Клемп позиции: допускаем вставку и "за правый край"
    if (pos > size_) {
        pos = size_;
    }
    EnsureCapacityForInsert(1);
    for (size_t i = size_; i > pos; --i) {
        new (data_ + i) T(std::move(data_[i - 1]));
        (data_ + (i - 1))->~T();
    }
    new (data_ + pos) T(value);
    ++size_;
    return data_ + pos;
}

template <typename T>
T* Vector<T>::Insert(size_t pos, T&& value) {
    if (pos > size_) {
        pos = size_;
    }
    EnsureCapacityForInsert(1);
    for (size_t i = size_; i > pos; --i) {
        new (data_ + i) T(std::move(data_[i - 1]));
        (data_ + (i - 1))->~T();
    }
    new (data_ + pos) T(std::move(value));
    ++size_;
    return data_ + pos;
}

template <typename T>
void Vector<T>::Erase(size_t first, size_t last) noexcept {
    // Клемпим границы в [0, size_]
    if (first > size_) {
        first = size_;
    }
    if (last > size_) {
        last = size_;
    }
    if (first >= last) {
        return;
    }

    // разрушить [first, last)
    DestroyRange(first, last);

    // сдвинуть хвост влево: из [last, size_) в начало дырки с сохранением порядка
    const size_t tail = size_ - last;
    for (size_t i = 0; i < tail; ++i) {
        new (data_ + (first + i)) T(std::move(data_[last + i]));
        (data_ + (last + i))->~T();
    }
    size_ -= (last - first);
}
