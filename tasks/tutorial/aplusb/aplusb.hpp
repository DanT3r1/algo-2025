#include <cstdint>
#include <iostream>

int Sum(int a, int b) {
    int64_t la = static_cast<int64_t>(a);
    int64_t lb = static_cast<int64_t>(b);
    int64_t result = la + lb;

    if (result > INT_MAX) {
        return INT_MAX;
    }

    if (result < INT_MIN) {
        return INT_MIN;
    }
    return static_cast<int>(result);
}
