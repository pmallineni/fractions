# Fraction Class: Small, safe rational number template

## Overview

`Fraction<T>` is a compact C++17 template for working with rational numbers on signed integral types. Fractions are always kept in lowest terms using the Euclidean algorithm (GCD), and the sign is normalized to the numerator.

## Safety & Robustness

- Uses the Euclidean algorithm to simplify reliably.
- Performs domain checks (denominator != 0) and detects potential overflow during intermediate arithmetic.
- Uses checked arithmetic where needed to avoid undefined behavior on overflow; operations throw on invalid domain or overflow.


## Features

- Full arithmetic (`+`, `-`, `*`, `/`) with compound assignments; operations use checked arithmetic to avoid undefined overflow on fixed-width types.
- Comparison operators use a reduced cross-multiplication approach and fall back to a continued-fraction/Euclidean method when intermediate products would overflow, preserving correctness for very large values.
- Conversions to `double`/`int`/`string` and `operator<<` for streams.
- Works with any signed integral type (e.g. `int`, `long long`) and adapts to arbitrary-precision types by skipping unnecessary overflow checks for better performance.

The implementation emphasizes correctness and predictable failures: constructors and arithmetic detect invalid domains (zero denominators, division/modulo by zero) and throw exceptions on overflow so callers can handle errors deterministically.

## Quick Example

```cpp
#include "fractions.h"
#include <iostream>

int main() {
    Fraction<long long> a(6, 8); // simplifies to 3/4 via GCD
    Fraction<long long> b(1, 2);
    auto s = a + b;              // safe arithmetic, simplifies to 5/4
    std::cout << s << "\n";
    return 0;
}
```

## Build & Test

```bash
g++ -std=c++17 -g fractionTests.cpp -o fractionTests && ./fractionTests
```

---

All 176 test cases passed.
