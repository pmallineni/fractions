#pragma once
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>

template < 
    typename T,
    typename = std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>
>
struct Fraction {
    private: 
        T _numerator;
        T _denominator;

        void simplify() {
            T gcd = std::gcd(_numerator, _denominator); 
            _numerator /= gcd;
            _denominator /= gcd;
            
            if (_denominator < 0) {
                _numerator = -_numerator;
                _denominator = -_denominator;
            }
        }
    public: 
        T numerator() const {return _numerator;}
        T denominator() const {return _denominator;}
    

        Fraction(T p_num, T p_denom) : _numerator(p_num), _denominator(p_denom) {
            if (p_denom == 0) throw std::invalid_argument("Denominator cannot be zero.\n");
            simplify();
        } 

        explicit Fraction(T value) : _numerator(value), _denominator(1) {
            // simplify(); // Any integer numerator divided by 1 is already simplified
        }

        Fraction operator+ (const Fraction& other) const {
            T lcm = std::lcm(_denominator, other.denominator());
            return Fraction(
                _numerator * (lcm / _denominator) + other.numerator() * (lcm / other.denominator()), 
                lcm);
        }
        Fraction operator+ (const T other) const {
            return Fraction(_numerator + other * _denominator, _denominator);
        }

        Fraction operator- (const Fraction& other) const {
            T lcm = std::lcm(_denominator, other.denominator());
            return  Fraction(
                _numerator * (lcm / _denominator) - other.numerator() * (lcm / other.denominator()), 
                lcm);
        }
        Fraction operator- (const T other) const {
            return Fraction(_numerator - other * _denominator, _denominator);
        }

        Fraction operator* (const Fraction& other) const {
            return Fraction(_numerator * other.numerator(), _denominator * other.denominator());
        }
        Fraction operator* (const T other) const {
            return Fraction(_numerator * other, _denominator);
        }

        Fraction operator/ (const Fraction& other) const {
            if (other.numerator() == 0) throw std::domain_error("Cannot divide by zero.\n");
            return Fraction(_numerator * other.denominator(), _denominator * other.numerator());

        }
        Fraction operator/ (const T other) const {
            if (other == 0) throw std::domain_error("Cannot divide by zero.\n");
            return Fraction(_numerator, _denominator * other);
        }

        Fraction operator-() const {
            return Fraction (-_numerator, _denominator);
        }

        std::string toString() const {
            std::ostringstream oss;
            oss << *this;
            return oss.str();
        }

        double toDouble() const {
            return static_cast<double> (_numerator) / _denominator;
        }

        template < 
            typename U,
            typename = std::enable_if_t<std::is_integral_v<U> && std::is_signed_v<U>>
        >
        explicit operator U() const {
            return _numerator / _denominator;
        }

        T toInt() const {
            return static_cast<T> (*this);
        }

        bool const isInteger() const noexcept {return _denominator == 1;}
        
        friend std::ostream& operator<< (std::ostream& out, const Fraction& frac) {
            if (frac.isInteger()) 
                out << frac.numerator();
            else
                out << frac.numerator() << '/' << frac.denominator();
            return out;
        }

        friend Fraction operator+(T lhs, const Fraction& rhs) {
            return rhs + lhs;
        }

        friend Fraction operator-(T lhs, const Fraction& rhs) {
            return -rhs + lhs;
        }

        friend Fraction operator*(T lhs, const Fraction& rhs) {
            return rhs * lhs;
        }

        friend Fraction operator/(T lhs, const Fraction& rhs) {
            return Fraction(lhs) / rhs;
        }

        Fraction& operator+=(const Fraction& other) {
            T lcm = std::lcm(_denominator, other.denominator());
            _numerator = _numerator * (lcm / _denominator) + other.numerator() * (lcm / other.denominator());
            _denominator = lcm;
            simplify();
            return *this;
        }

        Fraction& operator+=(const T other) {
            _numerator = _numerator + other * _denominator;
            simplify();
            return *this;
        }


        Fraction& operator-=(const Fraction& other) {
            T lcm = std::lcm(_denominator, other.denominator());
            _numerator = _numerator * (lcm / _denominator) - other.numerator() * (lcm / other.denominator());
            _denominator = lcm;
            simplify();
            return *this;
        }

        Fraction& operator-=(const T other) {
            _numerator = _numerator - other * _denominator;
            simplify();
            return *this;
        }
    
        Fraction& operator*=(const Fraction& other) {
            _numerator = _numerator * other.numerator();
            _denominator = _denominator * other.denominator();
            simplify();
            return *this;
        }

        Fraction& operator*=(const T other) {
            _numerator = _numerator * other;
            simplify();
            return *this;
        }

        Fraction& operator/=(const Fraction& other) {
            if (other.numerator() == 0) throw std::domain_error("Cannot divide by zero.\n");
            _numerator = _numerator * other.denominator();
            _denominator = _denominator * other.numerator();
            simplify();
            return *this;
        }

        Fraction& operator/=(const T other) {
            if (other == 0) throw std::domain_error("Cannot divide by zero.\n");
            _denominator = _denominator * other;
            simplify();
            return *this;
        }

        bool operator==(const Fraction& other) const {
            return _numerator == other.numerator() && _denominator == other.denominator(); // only works because Fraction is always simplified
            // return _numerator * other.denominator() == _denominator * other.numerator(); // possible overflow
        }

        bool operator!=(const Fraction& other) const {
            return !operator==(other);
        }

        bool operator>=(const Fraction& other) const {
            T denom_gcd = std::gcd(_denominator, other.denominator());
            return (_numerator) * (other.denominator() / denom_gcd) >= (_denominator / denom_gcd) * (other.numerator());
        }

        bool operator<=(const Fraction& other) const {
            T num_gcd = std::gcd(_numerator, other.numerator());
            T denom_gcd = std::gcd(_denominator, other.denominator());
            return (_numerator) * (other.denominator() / denom_gcd) <= (_denominator / denom_gcd) * (other.numerator());
        }

        bool operator<(const Fraction& other) const {
            T num_gcd = std::gcd(_numerator, other.numerator());
            T denom_gcd = std::gcd(_denominator, other.denominator());
            return (_numerator) * (other.denominator() / denom_gcd) < (_denominator / denom_gcd) * (other.numerator());
        }

        bool operator>(const Fraction& other) const {
            T num_gcd = std::gcd(_numerator, other.numerator());
            T denom_gcd = std::gcd(_denominator, other.denominator());
            return (_numerator) * (other.denominator() / denom_gcd) > (_denominator / denom_gcd) * (other.numerator());
        }
        
        bool operator==(const T other) const { return _numerator == other * _denominator; }
        bool operator!=(const T other) const { return !operator==(other); }
        bool operator< (const T other) const { return _numerator <  other * _denominator; }
        bool operator> (const T other) const { return _numerator >  other * _denominator; }
        bool operator<=(const T other) const { return _numerator <= other * _denominator; }
        bool operator>=(const T other) const { return _numerator >= other * _denominator; }

        friend bool operator==(const T lhs, const Fraction& rhs) { return rhs == lhs; }
        friend bool operator!=(const T lhs, const Fraction& rhs) { return rhs != lhs; }
        friend bool operator< (const T lhs, const Fraction& rhs) { return rhs >  lhs; }
        friend bool operator> (const T lhs, const Fraction& rhs) { return rhs <  lhs; }
        friend bool operator<=(const T lhs, const Fraction& rhs) { return rhs >= lhs; }
        friend bool operator>=(const T lhs, const Fraction& rhs) { return rhs <= lhs; }

};
