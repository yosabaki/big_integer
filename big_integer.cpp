#include "big_integer.h"

#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <cctype>
#include <stdexcept>

big_integer::big_integer() : sign(false), digits{0, 0} {
}

big_integer::big_integer(big_integer const &other) : digits(other.digits), sign(other.sign) {
}

big_integer::big_integer(unsigned int a) : sign(false), digits{a, 0} {}

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
        unsigned int t = 0;
        size_t j = 0;
        for (; j < 9 && i + j < str.size(); j++) {
            t *= 10;
            t += str[i + j] - '0';
        }
        *this = *this * int(pow(10, j));
        *this += t;
    }
    if (neg) *this = -*this;
}

//big_integer::~big_integer() {
//    digits.clear();
//}

big_integer &big_integer::operator=(big_integer const &other) {
    big_integer temp(other);
    std::swap(digits, temp.digits);
    std::swap(sign, temp.sign);
    return *this;
}

template<class FunctorT>
big_integer &big_integer::add(big_integer const &rhs, FunctorT f) {
    size_t size = std::max(this->size(), rhs.size()) + 1;
    unsigned int word = (rhs.sign ? f(UINT32_MAX) : f(0));
    this->digits.resize(size, (this->sign ? UINT32_MAX : 0));
    unsigned int tmp = 0;
    bool carry = f(0), tmpcarry;
    for (size_t i = 0; i < rhs.size(); i++) {
        tmpcarry = carry;
        unsigned int ri = f(rhs.digits[i]);
        tmp = this->digits[i] + ri;
        if (UINT32_MAX - this->digits[i] < ri || (!(~tmp) && carry)) {
            carry = 1;
        } else carry = 0;
        this->digits[i] = tmp + tmpcarry;
    }
    for (size_t i = rhs.size(); i < size; i++) {
        tmpcarry = carry;
        unsigned int li = this->digits[i];
        tmp = li + word;
        if (UINT32_MAX - li < word || !(~tmp) && carry) {
            carry = 1;
        } else carry = 0;
        this->digits[i] = tmp + tmpcarry;
    }
    this->sign = this->digits.back();
    this->delete_leading_zeros();
    return *this;
}


big_integer &big_integer::operator+=(big_integer const &rhs) {
    return add(rhs, element());
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    return add(rhs, std::bit_not<uint32_t>());
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    bool neg = this->sign ^rhs.sign;
    big_integer left = abs(*this), right = abs(rhs);
    *this = 0;
    for (size_t i = 0; i < rhs.size() - 1; i++) {
        *this += (left * right.digits[i]) << (32 * i);
    }
    this->delete_leading_zeros();
    if (neg) *this = -*this;
    return *this;
}

big_integer &big_integer::sub_from(big_integer const &rhs, int pos) {
    unsigned int word = (!rhs.sign ? UINT32_MAX : 0);
    unsigned int tmp = 0;
    char carry = 1, tmpcarry;
    for (size_t i = 0; i < rhs.size(); i++) {
        tmpcarry = carry;
        tmp = this->digits[i + pos] + ~rhs.digits[i];
        if (UINT32_MAX - this->digits[i + pos] < ~rhs.digits[i] || (!(~tmp) && carry)) {
            carry = 1;
        } else carry = 0;
        this->digits[i + pos] = tmp + tmpcarry;
    }
    for (size_t i = rhs.size() + pos; i < this->size(); i++) {
        tmpcarry = carry;
        unsigned int li = this->digits[i];
        tmp = li + word;
        if (UINT32_MAX - li < word || !(~tmp) && carry) {
            carry = 1;
        } else carry = 0;
        this->digits[i] = tmp + tmpcarry;
    }
    this->sign = this->digits.back();
    this->delete_leading_zeros();
    return *this;
}

big_integer big_integer::divide2n1n(big_integer &rhs) {
    big_integer left = *this;
    size_t r = rhs.size() - 1, l = this->size();
    unsigned int d = UINT32_MAX / (rhs.digits[r - 1] + 1);
    left *= d;
    rhs *= d;
    unsigned int b = rhs.digits[r - 1];
    l--;
    this->digits.resize(l - r + 1);
    for (int j = l - r; j > -1; j--) {
        unsigned long long a = (((unsigned long long) left.digits[j + r] << 32) + left.digits[j + r - 1]);
        unsigned long long qi = std::min(a / b, (unsigned long long) UINT32_MAX - 1);
        unsigned long long ri = a % b;
        while (ri < UINT32_MAX && qi * rhs.get_digit(r - 2) > (ri << 32) + left.digits[j + r - 2]) {
            qi--;
            ri += b;
        }
        left.sub_from(rhs * qi, j);
        this->digits[j] = (unsigned int) qi;
    }
    this->delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    bool neg = this->sign ^rhs.sign;
    big_integer right = abs(rhs);
    *this = abs(*this);
    if (rhs.is_zero()) {
        throw std::runtime_error("Division by zero");
    }
    if (*this < right) {
        *this = 0;
        return *this;
    }
    this->divide2n1n(right);
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
        this->digits[i] = functor(this->digits[i], rhs.get_digit(i));
    }
    sign = digits.back();
    this->delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    return this->bitwise_operation(rhs, std::bit_and<uint32_t>());
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    return this->bitwise_operation(rhs, std::bit_or<uint32_t>());
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    return this->bitwise_operation(rhs, std::bit_xor<uint32_t>());
}

void big_integer::delete_leading_zeros() {
    unsigned int word = (sign ? UINT32_MAX : 0);
    if (digits[size() - 1] != word) {
        digits.resize(size() + 1, word);
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
    int prev = rhs / 32, step = rhs % 32;
    size_t size = this->size() + prev + 1;
    this->digits.resize(size, this->sign ? UINT32_MAX : 0);
    for (size_t i = size - 1; i > prev; i--) {
        this->digits[i] = (this->digits[i - prev] << step) |
                          (unsigned int) ((unsigned long long) this->digits[i - prev - 1] >> (32 - step));
    }
    this->digits[prev] = this->digits[0] << step;
    for (int i = 0; i < prev; i++) {
        this->digits[i] = 0;
    }
    this->sign = this->digits.back();
    this->delete_leading_zeros();
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs < 0) return *this <<= (-rhs);
    int prev = rhs / 32, step = rhs % 32;
    int size = std::max((int) this->size() - prev, 2);
    for (int i = 0; i < size; i++) {
        this->digits[i] =
                (this->get_digit(i + prev + 1) << (32 - step)) | (this->digits[i + prev] >> step);
    }
    this->digits.resize(size);
    if (size == 0) this->digits.push_back(0);
    this->sign = this->digits.back();
    this->delete_leading_zeros();
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    return 0 - *this;
}

size_t big_integer::size() const {
    return this->digits.size();
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
    bool neg = false;
    if (a.sign) {
        neg = true;
        a = -a;
    }
    unsigned long long carry = 0;
    for (size_t i = 0; i < a.size(); i++) {
        unsigned long long tmp = (unsigned long long) b * a.digits[i] + carry;
        a.digits[i] = (unsigned int) tmp;
        carry = (tmp >> 32);
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
        unsigned int c = (tmp % int(1e9)).digits[0];
        tmp /= int(1e9);
        bool b = tmp.is_zero();
        for (int i = 0; i < 9; i++) {
            res = char(c % 10 + int('0')) + res;
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
