#pragma once
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace checked_arith_detail {

    // ---- Helper: is T an unbounded (arbitrary-precision) type? ----
    // True only if numeric_limits is specialized AND reports bounded.
    // Arbitrary-precision types either leave numeric_limits unspecialized,
    // or specialize it with is_bounded = false.
    template <typename T>
    inline constexpr bool is_bounded_v =
        std::numeric_limits<T>::is_specialized && std::numeric_limits<T>::is_bounded;


    // ================= checked_negate =================
    template <typename T>
    T checked_negate(T a) {
        if constexpr (!is_bounded_v<T>) {
            return -a; // unbounded: cannot overflow
        } else {
            // The only problematic case for signed bounded types: -min is not representable
            if (a == std::numeric_limits<T>::min())
                throw std::overflow_error("integer negation overflow");
            return -a;
        }
    }


    // ================= checked_add =================
    template <typename T>
    T checked_add(T a, T b) {
        if constexpr (!is_bounded_v<T>) {
            return a + b; // unbounded: cannot overflow
        }
#if defined(__GNUC__) || defined(__clang__)
        else {
            T result;
            if (__builtin_add_overflow(a, b, &result))
                throw std::overflow_error("integer addition overflow");
            return result;
        }
#else
        else {
            constexpr T max = std::numeric_limits<T>::max();
            constexpr T min = std::numeric_limits<T>::min();
            if ((b > 0 && a > max - b) || (b < 0 && a < min - b))
                throw std::overflow_error("integer addition overflow");
            return a + b;
        }
#endif
    }


    // ================= checked_sub =================
    template <typename T>
    T checked_sub(T a, T b) {
        if constexpr (!is_bounded_v<T>) {
            return a - b; // unbounded: cannot overflow
        }
#if defined(__GNUC__) || defined(__clang__)
        else {
            T result;
            if (__builtin_sub_overflow(a, b, &result))
                throw std::overflow_error("integer subtraction overflow");
            return result;
        }
#else
        else {
            constexpr T max = std::numeric_limits<T>::max();
            constexpr T min = std::numeric_limits<T>::min();
            if ((b < 0 && a > max + b) || (b > 0 && a < min + b))
                throw std::overflow_error("integer subtraction overflow");
            return a - b;
        }
#endif
    }


    // ================= checked_mul =================
    template <typename T>
    T checked_mul(T a, T b) {
        if constexpr (!is_bounded_v<T>) {
            return a * b; // unbounded: cannot overflow
        }
#if defined(__GNUC__) || defined(__clang__)
        else {
            T result;
            if (__builtin_mul_overflow(a, b, &result))
                throw std::overflow_error("integer multiplication overflow");
            return result;
        }
#else
        else {
            constexpr T max = std::numeric_limits<T>::max();
            constexpr T min = std::numeric_limits<T>::min();

            if (a == 0 || b == 0) return T(0);

            // -1 * min overflows (since -min is not representable)
            if ((a == T(-1) && b == min) || (b == T(-1) && a == min))
                throw std::overflow_error("integer multiplication overflow");

            if (a > 0) {
                if (b > 0) {
                    if (a > max / b) throw std::overflow_error("integer multiplication overflow");
                } else {
                    if (b < min / a) throw std::overflow_error("integer multiplication overflow");
                }
            } else {
                if (b > 0) {
                    if (a < min / b) throw std::overflow_error("integer multiplication overflow");
                } else {
                    if (a < max / b) throw std::overflow_error("integer multiplication overflow");
                }
            }
            return a * b;
        }
#endif
    }

    template <typename T>
    T checked_div(T a, T b) {
        if (b == 0)
            throw std::domain_error("integer division by zero");
        if constexpr (is_bounded_v<T>) {
            // INT_MIN / -1 overflows (result is -INT_MIN, unrepresentable)
            if (a == std::numeric_limits<T>::min() && b == T(-1))
                throw std::overflow_error("integer division overflow");
        }
        return a / b;
    }

    template <typename T>
    T checked_mod(T a, T b) {
        if (b == 0)
            throw std::domain_error("integer modulo by zero");
        if constexpr (is_bounded_v<T>) {
            if (a == std::numeric_limits<T>::min() && b == T(-1))
                throw std::overflow_error("integer modulo overflow");
        }
        return a % b;
    }


    // ================= CheckedInt wrapper =================
    template <typename T>
    class CheckedInt {
    public:
        constexpr CheckedInt() : value_(T(0)) {}
        constexpr CheckedInt(T v) : value_(v) {}  // implicit: allows CheckedInt<T> x = some_T;

        T value() const { return value_; }
        explicit operator T() const { return value_; } // explicit: forces deliberate unwrap

        CheckedInt operator+(const CheckedInt& o) const { return CheckedInt(checked_add(value_, o.value_)); }
        CheckedInt operator-(const CheckedInt& o) const { return CheckedInt(checked_sub(value_, o.value_)); }
        CheckedInt operator*(const CheckedInt& o) const { return CheckedInt(checked_mul(value_, o.value_)); }
        CheckedInt operator/(const CheckedInt& o) const { return CheckedInt(checked_div(value_, o.value_)); }
        CheckedInt operator%(const CheckedInt& o) const { return CheckedInt(checked_mod(value_, o.value_)); }
        CheckedInt operator-() const                    { return CheckedInt(checked_negate(value_)); }

        // Comparisons carry no overflow risk - forward directly
        bool operator==(const CheckedInt& o) const { return value_ == o.value_; }
        bool operator!=(const CheckedInt& o) const { return value_ != o.value_; }
        bool operator< (const CheckedInt& o) const { return value_ <  o.value_; }
        bool operator> (const CheckedInt& o) const { return value_ >  o.value_; }
        bool operator<=(const CheckedInt& o) const { return value_ <= o.value_; }
        bool operator>=(const CheckedInt& o) const { return value_ >= o.value_; }

    private:
        T value_;
    };

    // gcd/lcm operating on CheckedInt - division/modulo inside std::gcd-style
    // algorithms never overflow for positive operands shrinking toward 0,
    // but checked_mod/checked_div still guard the abs(min)/-1 edge case
    template <typename T>
    CheckedInt<T> int_gcd(CheckedInt<T> a, CheckedInt<T> b) {
        if (a < CheckedInt<T>(T(0))) a = -a;
        if (b < CheckedInt<T>(T(0))) b = -b;
        while (b != CheckedInt<T>(T(0))) {
            CheckedInt<T> t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    template <typename T>
    CheckedInt<T> int_lcm(CheckedInt<T> a, CheckedInt<T> b) {
        return (a / int_gcd(a, b)) * b;
    }

}

// A trait for "behaves like a signed integer" -- true for any built-in
// signed integers, __int128 (via specialization if needed), and any type
// for which the user specializes it (e.g. boost::multiprecision::cpp_int)
template <typename T>
struct is_integer_like : std::integral_constant<bool,
    std::is_integral_v<T> && std::is_signed_v<T>> {};

#if defined(__SIZEOF_INT128__)
template <> struct is_integer_like<__int128> : std::true_type {};
#endif

// integer_like objects defined in boost::multiprecision (e.g. for arbitrary precision integers)
#if defined(BOOST_MP_VERSION)
template <typename Backend, boost::multiprecision::expression_template_option ExpressionTemplates>
struct is_integer_like<boost::multiprecision::number<Backend, ExpressionTemplates>> 
    : std::integral_constant<bool, 
        boost::multiprecision::number_category<boost::multiprecision::number<Backend, ExpressionTemplates>>::value 
        == boost::multiprecision::number_kind_integer
    > {};
#endif

template <typename T>
inline constexpr bool is_integer_like_v = is_integer_like<T>::value;

template < 
    typename T,
    typename = std::enable_if_t<is_integer_like_v<T>>
>
struct Fraction {
    using C = checked_arith_detail::CheckedInt<T>;
    private: 
        T _numerator;
        T _denominator;

        void simplify() {
            T gcd = static_cast<T>(checked_arith_detail::int_gcd(C(_numerator), C(_denominator))); 
            // gcd never 0 when _denominator never 0
            _numerator /= gcd;
            _denominator /= gcd;
            
            if (_denominator < 0) {
                _numerator = checked_arith_detail::checked_negate(_numerator);
                _denominator = checked_arith_detail::checked_negate(_denominator);
            }
        }
    public: 
        T numerator()   const noexcept {return _numerator;}
        T denominator() const noexcept {return _denominator;}
    

        Fraction(T p_num, T p_denom) : _numerator(p_num), _denominator(p_denom) {
            if (p_denom == 0) throw std::invalid_argument("Denominator cannot be zero.\n");
            simplify();
        } 

        explicit Fraction(T value) : _numerator(value), _denominator(1) {
            // simplify(); // Any integer numerator divided by 1 is already simplified
        }

        // TODO for all + - * / operators
        // wrap try catch loop
        // checkedInt throws for overflow and divide by zero, wrap error message like "Fraction error: {e.what()}";
        Fraction operator+ (const Fraction& other) const {
            C ad{_denominator}, bd{other.denominator()};
            C lcm = checked_arith_detail::int_lcm(ad, bd);
            C num = C(_numerator) * (lcm / ad) + C(other.numerator()) * (lcm / bd);
            return Fraction(static_cast<T>(num), static_cast<T>(lcm));
        }
        Fraction operator+ (const T other) const {
            C num = C(_numerator) + C(other) * C(_denominator);
            return Fraction(static_cast<T>(num), _denominator);
        }

        Fraction operator- (const Fraction& other) const {
            C ad{_denominator}, bd{other.denominator()};
            C lcm = checked_arith_detail::int_lcm(ad, bd);
            C num = C(_numerator) * (lcm / ad) - C(other.numerator()) * (lcm / bd);
            return Fraction(static_cast<T>(num), static_cast<T>(lcm));
        }
        Fraction operator- (const T other) const {
            C num = C(_numerator) - C(other) * C(_denominator);
            return Fraction(static_cast<T>(num), _denominator);
        }

        Fraction operator* (const Fraction& other) const {
            C num = C(_numerator) * C(other.numerator());
            C denom = C(_denominator) * C(other.denominator());
            return Fraction(static_cast<T>(num), static_cast<T>(denom));
        }
        Fraction operator* (const T other) const {
            C num = C(_numerator) * C(other);
            return Fraction(static_cast<T>(num), _denominator);
        }

        Fraction operator/ (const Fraction& other) const {
            // TODO: try catch loop for overflow and domain_error (divide by zero checked_div throws)
            if (other.numerator() == 0) throw std::domain_error("Cannot divide by zero.");
            C num = C(_numerator) * C(other.denominator());
            C denom = C(_denominator) * C(other.numerator());          
            return Fraction(static_cast<T>(num), static_cast<T>(denom));

        }
        Fraction operator/ (const T other) const {
            if (other == 0) throw std::domain_error("Cannot divide by zero.");
            C denom = C(_denominator) * C(other);
            return Fraction(_numerator, static_cast<T>(denom));
        }

        Fraction operator-() const {
            return Fraction (checked_arith_detail::checked_negate(_numerator), _denominator);
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
            C ad{_denominator}, bd{other.denominator()};
            C lcm = checked_arith_detail::int_lcm(ad, bd);
            C num = C(_numerator) * (lcm / ad) + C(other.numerator()) * (lcm / bd);
            _numerator = static_cast<T>(num);
            _denominator = static_cast<T>(lcm);
            simplify();
            return *this;
        }

        Fraction& operator+=(const T other) {
            C num = C(_numerator) + C(other) * C(_denominator);
            _numerator = static_cast<T>(num);
            simplify();
            return *this;
        }


        Fraction& operator-=(const Fraction& other) {
            C ad{_denominator}, bd{other.denominator()};
            C lcm = checked_arith_detail::int_lcm(ad, bd);
            C num = C(_numerator) * (lcm / ad) - C(other.numerator()) * (lcm / bd);
            _numerator = static_cast<T>(num);
            _denominator = static_cast<T>(lcm);
            simplify();
            return *this;
        }

        Fraction& operator-=(const T other) {
            C num = C(_numerator) - C(other) * C(_denominator);
            _numerator = static_cast<T>(num);
            simplify();
            return *this;
        }
    
        Fraction& operator*=(const Fraction& other) {
            C num = C(_numerator) * C(other.numerator());
            C denom = C(_denominator) * C(other.denominator());
            _numerator = static_cast<T>(num);
            _denominator = static_cast<T>(denom);
            simplify();
            return *this;
        }

        Fraction& operator*=(const T other) {
            C num = C(_numerator) * C(other);
            _numerator = static_cast<T>(num);
            simplify();
            return *this;
        }

        Fraction& operator/=(const Fraction& other) {
            // TODO try catch loop 
            if (other.numerator() == 0) throw std::domain_error("Cannot divide by zero.");
            C num = C(_numerator) * C(other.denominator());
            C denom = C(_denominator) * C(other.numerator());
            _numerator = static_cast<T>(num);
            _denominator = static_cast<T>(denom);
            simplify();
            return *this;
        }

        Fraction& operator/=(const T other) {
            if (other == 0) throw std::domain_error("Cannot divide by zero.");
            C denom = C(_denominator) * C(other);
            _denominator = static_cast<T>(denom);
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

        bool operator<(const Fraction& other) const {
            C ad{_denominator}, bd{other.denominator()};
            C denom_gcd = checked_arith_detail::int_gcd(ad, bd);
            C lhs = C(_numerator) * (bd / denom_gcd);
            C rhs = C(other.numerator()) * (ad / denom_gcd);
            return lhs < rhs;
        }

        bool operator>(const Fraction& other) const {
            C ad{_denominator}, bd{other.denominator()};
            C denom_gcd = checked_arith_detail::int_gcd(ad, bd);
            C lhs = C(_numerator) * (bd / denom_gcd);
            C rhs = C(other.numerator()) * (ad / denom_gcd);
            return lhs > rhs;
        }

        bool operator>=(const Fraction& other) const {
            return *this == other || *this > other;
        }

        bool operator<=(const Fraction& other) const {
            return *this == other || *this < other;
        }
        
        bool operator==(const T other) const { return isInteger() && _numerator == other; }
        bool operator!=(const T other) const { return !operator==(other); }
        bool operator< (const T other) const { return _numerator <  static_cast<T>(C(other) * C(_denominator)); }
        bool operator> (const T other) const { return _numerator >  static_cast<T>(C(other) * C(_denominator)); }
        bool operator<=(const T other) const { return *this == other || *this < other; }
        bool operator>=(const T other) const { return *this == other || *this > other; }

        friend bool operator==(const T lhs, const Fraction& rhs) { return rhs == lhs; }
        friend bool operator!=(const T lhs, const Fraction& rhs) { return rhs != lhs; }
        friend bool operator< (const T lhs, const Fraction& rhs) { return rhs >  lhs; }
        friend bool operator> (const T lhs, const Fraction& rhs) { return rhs <  lhs; }
        friend bool operator<=(const T lhs, const Fraction& rhs) { return rhs >= lhs; }
        friend bool operator>=(const T lhs, const Fraction& rhs) { return rhs <= lhs; }

};
