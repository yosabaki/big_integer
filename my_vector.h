//
// Created by Artem Ustinov on 07.06.18.
//

#ifndef BIG_INTEGER_MY_VECTOR_H
#define BIG_INTEGER_MY_VECTOR_H


#include <cstdint>
#include <cstddef>
#include <variant>
#include <memory>

class my_vector {
public:

    void swap(my_vector &other);

    uint32_t &operator[](size_t const &_n);

    const uint32_t &operator[](size_t const &_n) const;

    void push_back(uint32_t _a);

    size_t size() const;

    my_vector();

    my_vector(my_vector const &other);

    explicit my_vector(size_t s);

    my_vector(size_t s, uint32_t _n);

    my_vector &operator=(my_vector const &other);

    void assign(size_t _n, uint32_t _a);

    void resize(size_t _n);

    void resize(size_t _n, uint32_t _a);

    uint32_t back();

    size_t capacity() const;

    ~my_vector();

private:
    void ensure_capacity(size_t _n);

    static const uint32_t _SIZE = 4;
    bool is_small;
    size_t _size;

    struct dynamic_data {
        std::shared_ptr<uint32_t[]> data;
        size_t capacity;

        dynamic_data() = default;

        dynamic_data(dynamic_data const &other) = default;

        dynamic_data(uint32_t other[], size_t c) : data(other), capacity(c) {}
    };

    union {
        dynamic_data big;
        uint32_t small[_SIZE];
    };
};


#endif //BIG_INTEGER_MY_VECTOR_H
