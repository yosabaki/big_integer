#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <iostream>

big_integer::big_integer() {
    digits.assign(2, 0);
    sign = 0;
}

big_integer::big_integer(big_integer const &other) {
    digits = other.digits;
    sign = other.sign;
}

big_integer::big_integer(unsigned int a) {
    digits.assign(2, 0);
    digits[0] = a;
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
    bool neg = 0;
    if (str[0] == '-') {
        neg = true;
    }
    for (size_t i = neg; i < str.size(); i++) {
        if (!isdigit(str[i])) {
            throw std::runtime_error("Invalid_string");
        }
    }
    big_integer tmp = 0;
    for (size_t i = neg; i < str.size(); i += 9) {
        unsigned int t = 0;
        size_t j = 0;
        for (; j < 9 && i + j < str.size(); j++) {
            t *= 10;
            t += str[i + j] - '0';
        }
        tmp *= int(pow(10, j));
        tmp += t;
    }
    digits = tmp.digits;
    if (neg) *this = -*this;
}

big_integer::~big_integer() {
    digits.clear();
}

big_integer &big_integer::operator=(big_integer const &other) {
    digits = other.digits;
    sign = other.sign;
    return *this;
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
    big_integer c;
    size_t size = std::max(this->size(), rhs.size()) + 1;
    c.digits.resize(size);
    long long tmp = 0;
    char carry = 0;
    for (size_t i = 0; i < size; i++) {
        tmp = (long long) this->get_digit(i) + rhs.get_digit(i) + carry;
        if (tmp > UINT32_MAX) {
            carry = 1;
            tmp &= UINT32_MAX;
        } else carry = 0;
        c.digits[i] = (unsigned int) tmp;
    }
    c.sign = (((c.digits.back() >> 31) & 1) == 1);
    *this = c;
    this->delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    return *this += (-rhs);
}

big_integer &big_integer::operator*=(big_integer const &rhs) {

    // find sign of result
    bool neg = this->sign ^rhs.sign;
    // do multiply operations with absolute values of arguments;
    big_integer left = abs(*this), right = abs(rhs);
    *this = 0;
    for (size_t i = 0; i < rhs.size() - 1; i++) {
        *this += (left * right.get_digit(i)) << (32 * i);
    }
    this->delete_leading_zeros();
    if (neg) *this = -*this;
    return *this;
}


big_integer big_integer::divide2n1n(big_integer const &rhs, big_integer &quontient) {
    quontient = 0;
    big_integer left = *this, right = rhs;
    size_t r = rhs.size() - 1, l = this->size();
    unsigned long long d = UINT32_MAX / (right.get_digit(r - 1) + 1);
    left *= (unsigned int) d;
    right *= (unsigned int) d;
    unsigned int b = right.get_digit(r - 1);
    l--;
    for (size_t j = l - r; j; j--) {
        unsigned long long a = (((unsigned long long) left.get_digit(j + r) << 32) + left.get_digit(j + r - 1));
        unsigned long long qi = std::min(a / b, (unsigned long long) UINT32_MAX - 1);
        unsigned long long ri = a % b;
        while (ri < UINT32_MAX && qi * right.get_digit(r - 2) > (ri << 32) + left.get_digit(j + r - 2)) {
            qi--;
            ri += b;
        }
        if ((left >> (32 * j)) < right * qi)
            qi--;
        left -= right * qi << (32 * j);
        big_integer tmp((unsigned int) qi);
        quontient += tmp << (j * 32);
    }
    return *this - quontient * rhs;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    // find sign of result
    bool neg = this->sign ^rhs.sign;
    // do divide operations with absolute values of arguments;
    big_integer left = abs(*this), right = abs(rhs);

    // check that divisor doesn't equal zero
    if (rhs.is_zero()) {
        throw std::runtime_error("Division by zero");
    }
    if (left < right) {
        *this = 0;
        return *this;
    }
    big_integer quontient;
    left.divide2n1n(right, quontient);
    *this = quontient;
    if (neg) *this = -*this;
    this->delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    // find sign of result
    bool neg = this->sign;
    // do module operations with absolute values of arguments;
    big_integer left = abs(*this), right = abs(rhs);

    // check that divisor doesn't equal zero
    if (rhs.is_zero()) {
        throw std::runtime_error("Module of zero");
    }
    if (left < right) {
        *this = left;
        return *this;
    }
    big_integer quontient;
    *this = left.divide2n1n(right, quontient);
    if (neg) *this = -*this;
    this->delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    for (size_t i = 0; i < size(); i++) {
        this->digits[i] = this->get_digit(i) & rhs.get_digit(i);
    }
    sign = (((digits.back() >> 31) & 1) == 1);
    this->delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    for (size_t i = 0; i < size(); i++) {
        this->digits[i] = this->get_digit(i) | rhs.get_digit(i);
    }
    sign = (((digits.back() >> 31) & 1) == 1);
    this->delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    for (size_t i = 0; i < size(); i++) {
        this->digits[i] = this->get_digit(i) ^ rhs.get_digit(i);
    }
    sign = (((digits.back() >> 31) & 1) == 1);
    this->delete_leading_zeros();
    return *this;
}

void big_integer::delete_leading_zeros() {
    unsigned int word = (sign ? UINT32_MAX : 0);
    if (digits[size() - 1] != word) {
        digits.push_back(word);
        return;
    }
    for (size_t i = size() - 1; i > 1; i--) {
        if (digits[i] == word && digits[i - 1] == word) {
            digits.pop_back();
        } else return;
    }
}

big_integer &big_integer::operator<<=(int rhs) {
    if (rhs < 0) return *this >>= (-rhs);
    big_integer c;
    int prev = rhs / 32;
    size_t size = this->size() + prev + 1;
    c.digits.resize(size);
    for (size_t i = prev; i < size; i++) {
        c.digits[i] = (this->get_digit(i - prev) << (rhs % 32)) |
                      (unsigned int) (((unsigned long long) this->get_digit(i - prev - 1) >> (32 - (rhs % 32))));
    }
    c.sign = (((c.digits.back() >> 31) & 1) == 1);
    *this = c;
    this->delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs < 0) return *this <<= (-rhs);
    big_integer c;
    int prev = rhs / 32;
    int size = std::max((int) this->size() - prev, 2);
    c.digits.resize(size);
    for (size_t i = 0; i < size; i++) {
        c.digits[i] = (this->get_digit(i + prev + 1) << (32 - (rhs % 32))) |
                      (unsigned int) (((unsigned long long) this->get_digit(i + prev) >> (rhs % 32)));
    }
    if (size == 0) c.digits.push_back(sign ? UINT32_MAX : 0);
    c.sign = (((c.digits.back() >> 31) & 1) == 1);
    *this = c;
    this->delete_leading_zeros();
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer r = *this;
    r = (~r) + 1;
    return r;
}

size_t big_integer::size() const {
    return this->digits.size();
}


big_integer big_integer::operator~() const {
    big_integer r;
    r.digits.resize(digits.size());
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

unsigned int big_integer::get_digit(size_t i) const {
    if (this->size() <= i) {
        return (this->sign ? UINT32_MAX : 0);
    } else return this->digits[i];
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

big_integer operator*(big_integer a, unsigned int const &b) {
    big_integer c;
    if (a.sign) {
        c.sign = true;
        a = -a;
    }
    c.digits.resize(a.size());
    unsigned int carry = 0;
    for (size_t i = 0; i < c.size(); i++) {
        unsigned long long tmp = (unsigned long long) b * a.get_digit(i) + carry;
        c.digits[i] = (unsigned int) tmp;
        carry = (unsigned int) (tmp >> 32);
    }
    if (carry) {
        c.digits.push_back(carry);
    }
    if (c.sign) c = -c;
    c.delete_leading_zeros();
    return c;
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
        if (is_zero()) {
            return 0;
        }
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
//f
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
        unsigned int c = (tmp % int(1e9)).get_digit(0);
        tmp /= int(1e9);
        bool b = tmp.is_zero();
        for (int i = 0; i < 9; i++) {
            res = char(char(c % 10) + '0') + res;
            c /= 10;
            if (b && c == 0) break;
        }
    }
    if (res == "") return "0";
    if (sign) res = '-' + res;
    return res;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    return s << to_string(a);
}