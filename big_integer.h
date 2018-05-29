#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <cstddef>
#include <vector>
#include <iosfwd>

struct big_integer {
    big_integer();

    big_integer(big_integer const &other);

    big_integer(unsigned int a);

    big_integer(int a);

    explicit big_integer(std::string const &str);

    ~big_integer();

    big_integer &operator=(big_integer const &other);

    big_integer &operator+=(big_integer const &rhs);

    big_integer &operator-=(big_integer const &rhs);

    big_integer &operator*=(big_integer const &rhs);

    big_integer &operator/=(big_integer const &rhs);

    big_integer &operator%=(big_integer const &rhs);

    big_integer &operator&=(big_integer const &rhs);

    big_integer &operator|=(big_integer const &rhs);

    big_integer &operator^=(big_integer const &rhs);

    big_integer &operator<<=(int rhs);

    big_integer &operator>>=(int rhs);

    big_integer operator+() const;

    big_integer operator-() const;

    big_integer operator~() const;

    big_integer &operator++();

    big_integer operator++(int);

    big_integer &operator--();

    big_integer operator--(int);

    friend bool operator==(big_integer const &a, big_integer const &b);

    friend bool operator!=(big_integer const &a, big_integer const &b);

    friend bool operator<(big_integer const &a, big_integer const &b);

    friend bool operator>(big_integer const &a, big_integer const &b);

    friend bool operator<=(big_integer const &a, big_integer const &b);

    friend bool operator>=(big_integer const &a, big_integer const &b);

    friend big_integer operator*(big_integer a, unsigned int const &b);

    friend std::string to_string(big_integer const &a);

    friend
    big_integer operator*(big_integer a, unsigned int const &b);

private:

    template<class FunctorT>
    big_integer &bitwise_operation(big_integer const &rhs, FunctorT functor);

    big_integer &sub_from(big_integer const &rhs, int pos);

    big_integer divide2n1n(big_integer &rhs);

    void delete_leading_zeros();

    bool is_zero() const;

    std::vector<unsigned int> digits;
    bool sign = 0;

    int cmp(big_integer const &b) const;

    unsigned int get_digit(size_t i) const;

    size_t size() const;
};


big_integer abs(big_integer const &a);

big_integer operator+(big_integer a, big_integer const &b);

big_integer operator-(big_integer a, big_integer const &b);

big_integer operator*(big_integer a, unsigned int const &b);

big_integer operator*(big_integer a, big_integer const &b);

big_integer operator/(big_integer a, big_integer const &b);

big_integer operator%(big_integer a, big_integer const &b);

big_integer operator&(big_integer a, big_integer const &b);

big_integer operator|(big_integer a, big_integer const &b);

big_integer operator^(big_integer a, big_integer const &b);

big_integer operator<<(big_integer a, int b);

big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const &a, big_integer const &b);

bool operator!=(big_integer const &a, big_integer const &b);

bool operator<(big_integer const &a, big_integer const &b);

bool operator>(big_integer const &a, big_integer const &b);

bool operator<=(big_integer const &a, big_integer const &b);

bool operator>=(big_integer const &a, big_integer const &b);

std::string to_string(big_integer const &a);

std::ostream &operator<<(std::ostream &s, big_integer const &a);

#endif // BIG_INTEGER_H