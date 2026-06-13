#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <numeric>   // std::gcd, std::lcm
#include <type_traits>
#include "fractions.h"

// ─── Test framework ───────────────────────────────────────────────────────────

struct TestResult {
    std::string name;
    bool        passed;
    std::string failure_message;
};

static std::vector<TestResult> g_results;

// Registers a pass or fail. Never throws — exceptions are caught per-test below.
static void check(bool condition, const std::string& test_name, const std::string& msg = "") {
    if (condition) {
        g_results.push_back({test_name, true, ""});
    } else {
        g_results.push_back({test_name, false, msg.empty() ? "condition was false" : msg});
    }
}

// Verifies that callable throws ExceptionType.
template <typename ExceptionType, typename Callable>
static void check_throws(Callable&& fn, const std::string& test_name) {
    try {
        fn();
        g_results.push_back({test_name, false, "expected exception was not thrown"});
    } catch (const ExceptionType&) {
        g_results.push_back({test_name, true, ""});
    } catch (...) {
        g_results.push_back({test_name, false, "wrong exception type was thrown"});
    }
}

// Verifies that callable does NOT throw.
template <typename Callable>
static void check_nothrow(Callable&& fn, const std::string& test_name) {
    try {
        fn();
        g_results.push_back({test_name, true, ""});
    } catch (const std::exception& e) {
        g_results.push_back({test_name, false, std::string("unexpected exception: ") + e.what()});
    } catch (...) {
        g_results.push_back({test_name, false, "unexpected unknown exception"});
    }
}

// ─── Individual test groups ───────────────────────────────────────────────────

using F =  Fraction<int>;
using FL = Fraction<long long>;

static void test_construction() {
    // Normal construction
    F a(1, 2);
    check(a.numerator() == 1 && a.denominator() == 2, "construct 1/2");

    // Simplification on construction
    F b(4, 8);
    check(b.numerator() == 1 && b.denominator() == 2, "simplify 4/8 → 1/2");

    F c(6, 4);
    check(c.numerator() == 3 && c.denominator() == 2, "simplify 6/4 → 3/2");

    F d(100, 25);
    check(d.numerator() == 4 && d.denominator() == 1, "simplify 100/25 → 4/1");

    // Single-argument (integer) construction
    F e(5);
    check(e.numerator() == 5 && e.denominator() == 1, "construct from integer 5");

    F z(0);
    check(z.numerator() == 0 && z.denominator() == 1, "construct from integer 0");

    // Zero numerator
    F f(0, 7);
    check(f.numerator() == 0 && f.denominator() == 1, "0/7 simplifies to 0/1");

    // Zero denominator throws
    check_throws<std::invalid_argument>([]{ F bad(1, 0); }, "zero denominator throws invalid_argument");
}

static void test_sign_normalization() {
    // Positive / positive
    F a(3, 4);
    check(a.numerator() > 0 && a.denominator() > 0, "3/4: both positive");

    // Negative numerator, positive denominator
    F b(-3, 4);
    check(b.numerator() == -3 && b.denominator() == 4, "-3/4: sign stays in numerator");

    // Positive numerator, negative denominator → sign moves to numerator
    F c(3, -4);
    check(c.numerator() == -3 && c.denominator() == 4, "3/-4 normalises to -3/4");

    // Both negative → positive fraction
    F d(-3, -4);
    check(d.numerator() == 3 && d.denominator() == 4, "-3/-4 normalises to 3/4");

    // Negative fraction that also simplifies
    F e(-6, -4);
    check(e.numerator() == 3 && e.denominator() == 2, "-6/-4 → 3/2");

    F f(-6, 4);
    check(f.numerator() == -3 && f.denominator() == 2, "-6/4 → -3/2");
}

static void test_addition() {
    // Same denominator
    F a(1, 4);
    F b(2, 4);
    check(a + b == F(3, 4), "1/4 + 2/4 = 3/4");

    // Different denominators
    F c(1, 3);
    F d(1, 6);
    check(c + d == F(1, 2), "1/3 + 1/6 = 1/2");

    // Adding integers
    check(F(1, 2) + 1 == F(3, 2),  "1/2 + 1 = 3/2");
    check(1 + F(1, 2) == F(3, 2),  "1 + 1/2 = 3/2 (friend)");

    // Negative addends
    check(F(1, 2) + F(-1, 2) == F(0), "1/2 + (-1/2) = 0");
    check(F(-1, 4) + F(-1, 4) == F(-1, 2), "-1/4 + -1/4 = -1/2");

    // Result needs simplification
    check(F(1, 6) + F(1, 6) == F(1, 3), "1/6 + 1/6 = 1/3");

    // Adding zero
    check(F(3, 7) + F(0) == F(3, 7), "adding zero identity");
}

static void test_subtraction() {
    check(F(3, 4) - F(1, 4) == F(1, 2), "3/4 - 1/4 = 1/2");
    check(F(1, 2) - F(1, 3) == F(1, 6), "1/2 - 1/3 = 1/6");
    check(F(1, 2) - 1       == F(-1, 2), "1/2 - 1 = -1/2");
    check(1       - F(1, 2) == F(1, 2),  "1 - 1/2 = 1/2 (friend)");
    check(F(1, 2) - F(1, 2) == F(0),     "self-subtraction = 0");

    // Subtracting a negative
    check(F(1, 4) - F(-1, 4) == F(1, 2), "1/4 - (-1/4) = 1/2");
}

static void test_multiplication() {
    check(F(2, 3) * F(3, 4) == F(1, 2), "2/3 * 3/4 = 1/2");
    check(F(1, 2) * 4       == F(2, 1), "1/2 * 4 = 2");
    check(4 * F(1, 2)       == F(2, 1), "4 * 1/2 = 2 (friend)");

    // Multiply by zero
    check(F(5, 7) * F(0)    == F(0),    "multiply by zero");
    check(F(5, 7) * 0       == F(0),    "multiply by integer 0");

    // Multiply by one
    check(F(3, 7) * F(1)    == F(3, 7), "multiply by 1 identity");

    // Negative
    check(F(-1, 2) * F(2, 3) == F(-1, 3), "-1/2 * 2/3 = -1/3");
    check(F(-1, 2) * F(-2, 3) == F(1, 3), "neg * neg = positive");
}

static void test_division() {
    check(F(1, 2) / F(1, 4) == F(2),    "1/2 ÷ 1/4 = 2");
    check(F(3, 4) / F(3, 4) == F(1),    "self-division = 1");
    check(F(1, 2) / 2       == F(1, 4), "1/2 ÷ 2 = 1/4");
    check(2       / F(1, 2) == F(4),    "2 ÷ 1/2 = 4 (friend)");
    check(F(2, 3) / F(4, 9) == F(3, 2), "2/3 ÷ 4/9 = 3/2");

    // Negative divisor
    check(F(1, 2) / F(-1, 4) == F(-2),  "1/2 ÷ (-1/4) = -2");

    // Division by zero (Fraction)
    check_throws<std::domain_error>([]{ (void)(F(1, 2) / F(0)); }, "Fraction / zero-fraction throws");

    // Division by zero (integer)
    check_throws<std::domain_error>([]{ (void)(F(1, 2) / 0); }, "Fraction / 0 (int) throws");

    // Compound /= by zero
    check_throws<std::domain_error>([]{ F f(1,2); f /= 0; }, "/= 0 (int) throws");
    check_throws<std::domain_error>([]{ F f(1,2); f /= F(0); }, "/= zero-fraction throws");
}

static void test_unary_minus() {
    check(-F(1, 2)  == F(-1, 2), "-( 1/2) = -1/2");
    check(-F(-1, 2) == F(1, 2),  "-(-1/2) =  1/2");
    check(-F(0)     == F(0),     "-(0) = 0");

    // Double negation
    F a(3, 5);
    check(-(-a) == a, "double negation is identity");
}

static void test_compound_assignment() {
    // +=
    F a(1, 4); a += F(1, 4);
    check(a == F(1, 2), "+= Fraction");

    F b(1, 2); b += 1;
    check(b == F(3, 2), "+= integer");

    // -=
    F c(3, 4); c -= F(1, 4);
    check(c == F(1, 2), "-= Fraction");

    F d(3, 2); d -= 1;
    check(d == F(1, 2), "-= integer");

    // *=
    F e(2, 3); e *= F(3, 4);
    check(e == F(1, 2), "*= Fraction");

    F ff(1, 2); ff *= 4;
    check(ff == F(2), "*= integer");

    // /=
    F g(1, 2); g /= F(1, 4);
    check(g == F(2), "/= Fraction");

    F h(3, 4); h /= 3;
    check(h == F(1, 4), "/= integer");

    // Chained +=
    F i(0);
    for (int k = 0; k < 4; ++k) i += F(1, 4);
    check(i == F(1), "four 1/4 additions equal 1");
}

static void test_comparisons_fraction_vs_fraction() {
    F half(1, 2);
    F third(1, 3);
    F also_half(2, 4);   // same value, different representation

    check(half   == also_half, "1/2 == 2/4");
    check(half   != third,     "1/2 != 1/3");
    check(third  <  half,      "1/3 < 1/2");
    check(half   >  third,     "1/2 > 1/3");
    check(half   >= also_half, "1/2 >= 2/4");
    check(half   <= also_half, "1/2 <= 2/4");
    check(third  <= half,      "1/3 <= 1/2");
    check(half   >= third,     "1/2 >= 1/3");

    // Negatives
    F neg_half(-1, 2);
    check(neg_half < half,   "-1/2 < 1/2");
    check(neg_half < third,  "-1/2 < 1/3");
    check(neg_half < F(0),   "-1/2 < 0");
    check(F(0)    > neg_half, "0 > -1/2");

    // Both zero
    check(F(0) == F(0, 5), "0/1 == 0/5 after simplification");
    check(F(0) >= F(0),    "0 >= 0");
    check(F(0) <= F(0),    "0 <= 0");
}

static void test_comparisons_fraction_vs_integer() {
    F a(4, 2);    // = 2
    check(a == 2, "4/2 == 2");
    check(a != 3, "4/2 != 3");
    check(a >  1, "4/2 > 1");
    check(a <  3, "4/2 < 3");
    check(a >= 2, "4/2 >= 2");
    check(a <= 2, "4/2 <= 2");

    // Reversed (friend operators)
    check(2 == a, "2 == 4/2 (friend)");
    check(3 != a, "3 != 4/2 (friend)");
    check(3 >  a, "3 > 4/2 (friend)");
    check(1 <  a, "1 < 4/2 (friend)");

    F b(1, 3);
    check(b < 1,  "1/3 < 1");
    check(0 < b,  "0 < 1/3 (friend)");
}

static void test_tostring_and_stream() {
    F a(3, 4);
    check(a.toString() == "3/4", "toString 3/4");

    F b(-1, 2);
    check(b.toString() == "-1/2", "toString -1/2");

    // operator<<
    std::ostringstream oss;
    oss << F(5, 3);
    check(oss.str() == "5/3", "operator<< produces 5/3");

    // toString and operator<< agree
    F c(7, 9);
    std::ostringstream oss2;
    oss2 << c;
    check(c.toString() == oss2.str(), "toString == operator<< output");

    std::ostringstream oss3;
    oss3 << F(8, 1);
    check(oss3.str() == "8", "operator<< produces integer 8 when fraction is integer");
}

static void test_conversions() {
    // toDouble
    check(F(1, 4).toDouble() == 0.25, "toDouble 1/4 = 0.25");
    check(F(1, 3).toDouble() == (1.0 / 3.0), "toDouble 1/3 = 1.0/3.0");
    check(F(-1, 2).toDouble() == -0.5, "toDouble -1/2 = -0.5");

    // toInt truncates toward zero
    check(F(7, 2).toInt() == 3,  "toInt 7/2 = 3 (truncate)");
    check(F(1, 2).toInt() == 0,  "toInt 1/2 = 0 (truncate)");
    check(F(-7, 2).toInt() == -3, "toInt -7/2 = -3 (truncate)");
    check(F(4, 2).toInt() == 2,  "toInt 4/2 = 2 (exact)");

    // static_cast conversions
    check(static_cast<int>(F(12, 3)) == 4, "static_cast<int>(12/3) = 4 (exact)");
    check(static_cast<short>(F(-9/4)) == short {-2}, "static_cast<short>(-9/4) = short {-2} (truncate)");
    
    // Convertability
    check(!std::is_constructible_v<float, F>, "Non-integral types not constructible from Fraction");
    check(!std::is_constructible_v<uint, F>, "Unsigned integral types not constructible from Fraction");
    check(std::is_constructible_v<long long, F>,
    "Signed integral types (long long) can be constructed from Fraction");
    check(std::is_constructible_v<int, F>, "Integral types (int) can be constructed from Fraction");
}

static void test_algebraic_identities() {
    F a(3, 7);
    F b(5, 11);
    F c(2, 9);
    F d(23,1);

    // Commutativity of + and *
    check(a + b == b + a, "addition commutative");
    check(a * b == b * a, "multiplication commutative");

    // Associativity of + and *
    check((a + b) + c == a + (b + c), "addition associative");
    check((a * b) * c == a * (b * c), "multiplication associative");

    // Distributivity
    check(a * (b + c) == a*b + a*c, "distributive law");

    // Additive inverse
    check(a + (-a) == F(0), "additive inverse");

    // Multiplicative inverse
    check(a * (F(1) / a) == F(1), "multiplicative inverse");

    // Subtraction as addition of negation
    check(a - b == a + (-b), "subtraction is addition of negation");

    // Fraction is integer when denominator is one
    check(d.isInteger(), "23/1 is an integer");
}

static void test_long_long() {
    FL a(1LL, 3LL);
    FL b(1LL, 6LL);
    check(a + b == FL(1LL, 2LL), "long long: 1/3 + 1/6 = 1/2");
    check(a * b == FL(1LL, 18LL), "long long: 1/3 * 1/6 = 1/18");
    // check type after static_cast

    check(static_cast<long long>(FL(2839473983389LL, 41LL)) == 69255463009,
    "static_cast<long long>(a/b) works for large numbers");

    // Large values that would overflow int
    FL big(1'000'000'000LL, 999'999'999LL);
    check_nothrow([&]{ (void)(big + FL(1LL)); }, "long long large value addition doesn't throw");
}

static void test_edge_cases() {
    // Fraction equal to whole number
    F whole(6, 3);
    check(whole.numerator() == 2 && whole.denominator() == 1, "6/3 → 2/1");
    check(whole == F(2), "6/3 == integer 2");

    // 1/1
    F one(1, 1);
    check(one == F(1), "1/1 == 1");

    // Negative whole
    F neg_whole(-9, 3);
    check(neg_whole.numerator() == -3 && neg_whole.denominator() == 1, "-9/3 → -3/1");

    // Large GCD simplification
    F large(1000, 750);
    check(large.numerator() == 4 && large.denominator() == 3, "1000/750 → 4/3");

    // Adding a fraction to its negation yields exactly zero
    F x(123, 456);
    check(x + (-x) == F(0), "x + (-x) = 0 for arbitrary fraction");

    // Dividing by self yields exactly 1
    F y(7, 13);
    check(y / y == F(1), "x / x = 1 for arbitrary fraction");
}

// ─── Runner ───────────────────────────────────────────────────────────────────

void runTests() {
    test_construction();
    test_sign_normalization();
    test_addition();
    test_subtraction();
    test_multiplication();
    test_division();
    test_unary_minus();
    test_compound_assignment();
    test_comparisons_fraction_vs_fraction();
    test_comparisons_fraction_vs_integer();
    test_tostring_and_stream();
    test_conversions();
    test_algebraic_identities();
    test_long_long();
    test_edge_cases();

    int passed = 0, failed = 0;
    for (const auto& r : g_results) {
        if (r.passed) {
            ++passed;
        } else {
            ++failed;
            std::cout << "[FAIL] " << r.name;
            if (!r.failure_message.empty())
                std::cout << " — " << r.failure_message;
            std::cout << '\n';
        }
    }

    std::cout << "\n────────────────────────────────\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed"
              << " (out of " << g_results.size() << " tests)\n";

    if (failed == 0)
        std::cout << "All tests passed.\n";
}

int main() {
    runTests();
    return 0;
}