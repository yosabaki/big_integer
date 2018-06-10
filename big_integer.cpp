#include "big_integer.h"

#include <cstring>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <stdexcept>


big_integer::big_integer() : sign(false), digits(2, 0) {
}

big_integer::big_integer(big_integer const &other) : digits(other.digits), sign(other.sign) {
}

big_integer::big_integer(uint32_t a) : sign(false), digits(2) {
    digits[0] = a;
    digits[1] = 0;
}

big_integer::big_integer(int a) {
    digits.assign(2, 0);
    digits[0] = a;
    if (a < 0) {
        digits[1] = UINT32_MAX;
        sign = true;
    }
}

big_integer::big_integer(std::string const &str) {
    bool neg = false;
    if (str[0] == '-') {
        neg = true;
    }
    for (size_t i = neg; i < str.size(); i++) {
        if (!isdigit(str[i])) {
            throw std::runtime_error("Invalid_string");
        }
    }
    digits.resize(2, 0);
    for (size_t i = neg; i < str.size(); i += 9) {
        uint32_t t = 0;
        size_t j = 0;
        for (; j < 9 && i + j < str.size(); j++) {
            t *= 10;
            t += str[i + j] - '0';
        }
        *this = *this * static_cast<int>(pow(10, j));
        *this += t;
    }
    if (neg) *this = -*this;
}

//big_integer::~big_integer() {
//    digits.clear();
//}

big_integer &big_integer::operator=(big_integer const &other) {
    big_integer temp(other);
    digits.swap(temp.digits);
    std::swap(sign, temp.sign);
    return *this;
}

template<class FunctorT>
big_integer &big_integer::add(big_integer const &rhs, FunctorT f) {
    size_t size = std::max(this->size(), rhs.size()) + 1;
    uint32_t word = (rhs.sign ? f(UINT32_MAX) : f(0));
    digits.resize(size, (sign ? UINT32_MAX : 0));
    uint64_t tmp = 0;
    bool carry = f(0);
    for (size_t i = 0; i < rhs.size(); i++) {
        uint32_t ri = f(rhs.digits[i]);
        tmp = static_cast<uint64_t>(digits[i]) + ri + carry;
        digits[i] = static_cast<uint32_t >(tmp);
        carry = static_cast<bool>(tmp >> 32u);
    }
    for (size_t i = rhs.size(); i < size; i++) {
        uint32_t li = digits[i];
        tmp = static_cast<uint64_t>(li) + word + carry;
        digits[i] = static_cast<uint32_t >(tmp);
        carry = static_cast<bool>(tmp >> 32u);
    }
    sign = static_cast<bool>(digits.back());
    delete_leading_zeros();
    return *this;
}


big_integer &big_integer::operator+=(big_integer const &rhs) {
    return add(rhs, element());
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    return add(rhs, std::bit_not<>());
}


big_integer &big_integer::operator*=(big_integer const &rhs) {
    bool neg = sign ^rhs.sign;
    big_integer left = abs(*this), right = abs(rhs);
    digits.assign(size() + right.size(), 0);
    sign = 0;
    for (size_t i = 0; i < left.size(); i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < right.size(); j++) {
            uint64_t tmp = static_cast<uint64_t>(left.digits[i]) * right.digits[j];
            tmp += digits[i + j] + carry;
            digits[i + j] = static_cast<uint32_t >(tmp);
            carry = static_cast<uint32_t >(tmp >> 32u);
        }
        digits[i + right.size()] = static_cast<uint32_t>(carry);
    }
    delete_leading_zeros();
    if (neg) *this = -*this;
    return *this;
}

big_integer &big_integer::sub_from(big_integer const &rhs, int pos) {
    uint32_t word = (!rhs.sign ? UINT32_MAX : 0);
    uint64_t tmp = 0;
    char carry = 1;
    digits.resize(std::max(rhs.size()+pos,size()),0-sign);
    for (size_t i = 0; i < rhs.size(); i++) {
        tmp = static_cast<uint64_t>(digits[i + pos]) + ~rhs.digits[i] + carry;
        digits[i + pos] = static_cast<uint32_t >(tmp);
        carry = static_cast<bool>(tmp >> 32u);
    }
    for (size_t i = rhs.size() + pos; i < size(); i++) {
        uint32_t li = digits[i];
        tmp = static_cast<uint64_t>(li) + word + carry;
        digits[i] = static_cast<uint32_t >(tmp);
        carry = static_cast<bool>(tmp >> 32u);
    }
    sign = static_cast<bool>(digits.back());
    delete_leading_zeros();
    return *this;
}

big_integer big_integer::divide2n1n(big_integer &rhs) {
    big_integer left = *this;
    size_t r = rhs.size() - 1, l = size();
    uint32_t d = UINT32_MAX / (rhs.digits[r - 1] + 1);
    left *= d;
    rhs *= d;
    uint32_t b = rhs.digits[r - 1];
    l--;
    digits.resize(l - r + 1);
    for (int j = l - r; j > -1; j--) {
        uint64_t a = ((static_cast<uint64_t> (left.get_digit(j + r)) << 32u) + left.get_digit(j + r - 1));
        uint64_t qi = std::min(a / b, static_cast<uint64_t> (UINT32_MAX));
        uint64_t ri = a % b;
        while (ri < UINT32_MAX && qi * rhs.get_digit(r - 2) > (ri << 32u) + left.get_digit(j + r - 2)) {
            qi--;
            ri += b;
        }
        left.sub_from(rhs * qi, j);
        digits[j] = static_cast<uint32_t >(qi);
    }
    delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    bool neg = sign ^rhs.sign;
    big_integer right = abs(rhs);
    *this = abs(*this);
    if (rhs.is_zero()) {
        throw std::runtime_error("Division by zero");
    }
    if (*this < right) {
        *this = 0;
        return *this;
    }
    divide2n1n(right);
    if (neg) *this = -*this;
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    if (rhs.is_zero()) {
        throw std::runtime_error("Module of zero");
    }
    return *this -= (*this / rhs) * rhs;
}

template<class FunctorT>
big_integer &big_integer::bitwise_operation(big_integer const &rhs, FunctorT functor) {
    for (size_t i = 0; i < size(); i++) {
        digits[i] = functor(digits[i], rhs.get_digit(i));
    }
    sign = static_cast<bool>(digits.back());
    delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    return bitwise_operation(rhs, std::bit_and<>());
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    return bitwise_operation(rhs, std::bit_or<>());
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    return bitwise_operation(rhs, std::bit_xor<>());
}

void big_integer::delete_leading_zeros() {
    uint32_t word = (sign ? UINT32_MAX : 0);
    if (digits.back() != word) {
        digits.push_back(word);
        return;
    }
    size_t true_size = size();
    for (size_t i = size() - 1; i > 1 && digits[i - 1] == word; i--) {
        true_size--;
    }
    digits.resize(true_size);
}

big_integer &big_integer::operator<<=(int rhs) {
    if (rhs < 0) return *this >>= (-rhs);
    uint32_t prev = rhs / 32, step = rhs % 32;
    size_t size = this->size() + prev + 1;
    digits.resize(size, sign ? UINT32_MAX : 0);
    for (size_t i = size - 1; i > prev; i--) {
        digits[i] = (digits[i - prev] << step) |
                    static_cast<uint32_t >(static_cast<uint64_t > (digits[i - prev - 1]) >> (32 - step));
    }
    digits[prev] = digits[0] << step;
    for (int i = 0; i < prev; i++) {
        digits[i] = 0;
    }
    sign = static_cast<bool>(digits.back());
    delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs < 0) return *this <<= (-rhs);
    uint32_t prev = rhs / 32, step = rhs % 32;
    size_t size = std::max(this->size() - prev, static_cast<size_t>(2));
    for (size_t i = 0; i < size; i++) {
        digits[i] =
                (get_digit(i + prev + 1) << (32 - step)) | (digits[i + prev] >> step);
    }
    digits.resize(size);
    if (size == 0) digits.push_back(0);
    sign = static_cast<bool>(digits.back());
    delete_leading_zeros();
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    return ~*this + 1;
}

size_t big_integer::size() const {
    return digits.size();
}


big_integer big_integer::operator~() const {
    big_integer r;
    r.digits.resize(size());
    for (size_t i = 0; i < r.size(); i++)
        r.digits[i] = ~digits[i];
    r.sign = !sign;
    return r;
}

big_integer &big_integer::operator++() {
    *this += 1;
    return *this;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer &big_integer::operator--() {
    *this -= 1;
    return *this;
}

big_integer big_integer::operator--(int) {
    big_integer r = *this;
    --*this;
    return r;
}

uint32_t big_integer::get_digit(size_t i) const {
    if (size() <= i) {
        return (sign ? UINT32_MAX : 0);
    } else return digits[i];
}


big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}


big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator*(big_integer a, uint32_t const &b) {
    bool neg = false;
    if (a.sign) {
        neg = true;
        a = -a;
    }
    uint32_t carry = 0;
    for (size_t i = 0; i < a.size(); i++) {
        uint64_t tmp = static_cast<uint64_t> (b) * a.digits[i] + carry;
        a.digits[i] = (uint32_t) tmp;
        carry = static_cast<uint32_t >(tmp >> 32u);
    }
    if (neg) a = -a;
    return a;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

bool big_integer::is_zero() const {
    if (sign) return false;
    for (size_t i = 0; i < size(); i++) {
        if (digits[i] != 0) {
            return false;
        }
    }
    return true;
}

int big_integer::cmp(big_integer const &b) const {
    if (b.is_zero()) {
        if (is_zero())
            return 0;
        return (sign ? -1 : 1);
    }
    big_integer tmp = *this - b;
    if (tmp.is_zero()) return 0;
    return (tmp.sign ? -1 : 1);
}

big_integer abs(big_integer const &a) {
    return (a < 0 ? -a : a);
}

bool operator==(big_integer const &a, big_integer const &b) {
    return a.cmp(b) == 0;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
    return a.cmp(b) < 0;
}

bool operator>(big_integer const &a, big_integer const &b) {
    return a.cmp(b) > 0;
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return !(a > b);
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(a < b);
}

std::string to_string(big_integer const &a) {
    std::string res;
    big_integer tmp = a;
    bool sign = false;
    if (tmp.sign) {
        sign = true;
        tmp = -tmp;
    }
    while (!tmp.is_zero()) {
        uint32_t c = (tmp % int(1e9)).digits[0];
        tmp /= int(1e9);
        bool b = tmp.is_zero();
        for (int i = 0; i < 9; i++) {
            res = char(c % 10 + int('0')) + res;
            c /= 10;
            if (b && c == 0) break;
        }
    }
    if (res.empty()) return "0";
    if (sign) res = '-' + res;
    return res;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    return s << to_string(a);
}
