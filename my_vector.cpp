//
// Created by Artem Ustinov on 07.06.18.
//

#include <cstring>
#include <algorithm>
#include <assert.h>
#include "my_vector.h"

my_vector::my_vector() : _size(0), is_small(1) {}

my_vector::my_vector(size_t s) : _size(s) {
    if (s > _SIZE) {
        new(&big) dynamic_data(new uint32_t[2 * s], 2 * s);
        is_small = false;
    } else is_small = true;
}

my_vector::my_vector(size_t s, uint32_t _n) : _size(s) {
    if (s > _SIZE) {
        auto tmp = new uint32_t[2 * s];
        std::memset(tmp, _n, s * sizeof(uint32_t));
        new(&big) dynamic_data(tmp, 2 * s);
        is_small = false;
    } else {
        is_small = true;
        std::memset(small, _n, s * sizeof(uint32_t));
    }
}

my_vector::my_vector(my_vector const &other) {
    _size = other._size;
    is_small = other.is_small;
    if (other.is_small) {
        std::memcpy(small, other.small, _SIZE * sizeof(uint32_t));
    } else {
        new(&big) dynamic_data(other.big);
    }
}

size_t my_vector::size() const {
    return _size;
}

size_t my_vector::capacity() const {
    return (is_small ? _SIZE : big.capacity);
}

void my_vector::resize(size_t _n) {
    ensure_capacity(_n);
    uint32_t *ptr = (is_small ? small : big.data.get());
    for (; _size < _n; _size++)
        ptr[_size] = 0;
    _size = _n;
}

void my_vector::resize(size_t _n, uint32_t _a) {
    ensure_capacity(_n);
    uint32_t *ptr = (is_small ? small : big.data.get());
    for (; _size < _n; _size++)
        ptr[_size] = _a;
    _size = _n;
}

void my_vector::push_back(uint32_t _a) {
    resize(_size + 1, _a);
}

uint32_t my_vector::back() {
    return (is_small ? small[_size - 1] : big.data[_size - 1]);
}

void my_vector::assign(size_t _n, uint32_t _a) {
    ensure_capacity(_n);
    uint32_t *ptr = (is_small ? small : big.data.get());
    for (_size = 0; _size < _n; _size++)
        ptr[_size] = _a;
}

uint32_t &my_vector::operator[](size_t const &_n) {
    assert(_n < _size);
    if (!is_small) {
        if (!big.data.unique()) {
            auto tmp = new uint32_t[capacity()];
            memcpy(tmp, big.data.get(), _size * sizeof(uint32_t));
            big.data.reset(tmp);
        }
        return big.data[_n];
    }
    return small[_n];
}

const uint32_t &my_vector::operator[](size_t const &_n) const {
    if (is_small)
        return small[_n];
    return big.data[_n];
}

void my_vector::swap(my_vector &other) {
    if (is_small) {
        if (other.is_small) {
            std::swap(small, other.small);
        } else {
            uint32_t tmp[_SIZE];
            memcpy(tmp, small, _size * sizeof(uint32_t));
            new(&big) dynamic_data(other.big);
            other.big.~dynamic_data();
            memcpy(other.small, tmp, _size * sizeof(uint32_t));
        }
    } else {
        if (other.is_small) {
            uint32_t tmp[_SIZE];
            memcpy(tmp, other.small, other._size * sizeof(uint32_t));
            new(&other.big) dynamic_data(big);
            big.~dynamic_data();
            memcpy(small, tmp, other._size * sizeof(uint32_t));
        } else {
            std::swap(big, other.big);
        }
    }
    std::swap(_size, other._size);
    std::swap(is_small, other.is_small);
}

my_vector &my_vector::operator=(my_vector const &other) {
    my_vector tmp(other);
    swap(tmp);
    return *this;
}

my_vector::~my_vector() {
    if (!is_small)
        big.~dynamic_data();
}

void my_vector::ensure_capacity(size_t _n) {
    if (_n <= _SIZE) {
        if (!is_small) {
            is_small = true;
            uint32_t tmp[_SIZE];
            memcpy(tmp, big.data.get(), _n * sizeof(uint32_t));
            big.~dynamic_data();
            memcpy(small, tmp, _n * sizeof(uint32_t));
        }
        return;
    }
    if (_n > _SIZE) {
        if (is_small) {
            is_small = false;
            auto tmp = new uint32_t[_n * 2 + 1];
            memcpy(tmp, small, _size * sizeof(uint32_t));
            new(&big) dynamic_data(tmp, _n * 2 + 1);
            return;
        }
    }
    if ((capacity() <= _n) || (_n * 4 < capacity())) {
        auto tmp = new uint32_t[_n * 2 + 1];
        memcpy(tmp, big.data.get(), std::min(_size, _n) * sizeof(uint32_t));
        big.~dynamic_data();
        new(&big) dynamic_data(tmp, _n * 2 + 1);
        return;
    }
    if (!big.data.unique()) {
        auto tmp = new uint32_t[capacity()];
        memcpy(tmp, big.data.get(), _size * sizeof(uint32_t));
        big.data.reset(tmp);
    }
}


