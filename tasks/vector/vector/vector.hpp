#pragma once

#include <cstddef>
#include <initializer_list>
#include <new>
#include <utility>

template <typename T>
class Vector {
public:
    // ===== ctors / dtors =====
    Vector();
    Vector(size_t count, const T& value);
    Vector(const Vector& other);
    Vector(Vector&& other) noexcept;
    Vector(std::initializer_list<T> init);
    ~Vector();

    // ===== assign / swap =====
    Vector& operator=(const Vector& other);
    Vector& operator=(Vector&& other) noexcept;
    void Swap(Vector& other) noexcept;

    // ===== capacity =====
    size_t Size() const noexcept {
        return size_;
    }
    size_t Capacity() const noexcept {
        return capacity_;
    }
    bool Empty() const noexcept {
        return size_ == 0;
    }
    // alias для совместимости
    bool IsEmpty() const noexcept {
        return Empty();
    }

    void Reserve(size_t new_cap);

    // Resize без значения
    void Resize(size_t new_size);
    // Resize с заполнением фиксированным значением
    void Resize(size_t new_size, const T& value);
    void Resize(size_t new_size, T&& value);

    void Clear() noexcept;

    // ===== element access (ВСЕГДА ссылки) =====
    T& operator[](size_t idx) noexcept {
        return data_[idx];
    }
    const T& operator[](size_t idx) const noexcept {
        return data_[idx];
    }

    T& Front() noexcept {
        return data_[0];
    }
    const T& Front() const noexcept {
        return data_[0];
    }

    T& Back() noexcept {
        return data_[size_ - 1];
    }
    const T& Back() const noexcept {
        return data_[size_ - 1];
    }

    // ===== modifiers =====
    void PushBack(const T& value);
    void PushBack(T&& value);  // rvalue-перегрузка

    template <class... Args>
    T& EmplaceBack(Args&&... args);

    void PopBack() noexcept;

    // вставка одного элемента перед позицией pos
    T* Insert(size_t pos, const T& value);
    T* Insert(size_t pos, T&& value);

    // удаление полуинтервала [first, last)
    void Erase(size_t first, size_t last) noexcept;

    // сырой доступ к данным (как в тестах)
    T* Data() noexcept {
        return data_;
    }
    const T* Data() const noexcept {
        return data_;
    }

private:
    // helpers
    void DestroyRange(size_t from, size_t to) noexcept;  // [from, to)
    void ReallocateAndMove(size_t new_cap);
    void EnsureCapacityForInsert(size_t add);
    static size_t GrowCapacity(size_t cur, size_t need);

private:
    T* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
};
