# Fraction Class: Generic Rational Number Library

## Overview

A C++17 template class `Fraction<T>` for working with rational numbers using signed integral types. Every fraction automatically simplifies to lowest terms via GCD and normalizes sign to the numerator. Supports full arithmetic (`+`, `-`, `*`, `/`), comparisons, conversions to double/int/string, and seamless scalar mixing (e.g., `Fraction(1,2) + 3`).

## Features

Arithmetic and comparison operators with automatic simplification, compound assignment operators (`+=`, `-=`, `*=`, `/=`), stream output via `operator<<`, and exception handling for invalid denominators and division by zero. Works with any signed integral type, including `int` and `long long`. Preserves algebraic identities (commutativity, associativity, distributivity).

## Quick Example

```cpp
#include "fractions.h"
#include <iostream>

int main() {
    // Construction and simplification
    Fraction<int> f1(6, 8);      // Automatically becomes 3/4
    Fraction<int> f2(1, 2);
    
    // Arithmetic
    Fraction<int> sum = f1 + f2;  // 3/4 + 1/2 = 5/4
    Fraction<int> product = f1 * 2; // 3/4 * 2 = 3/2
    
    // Comparison
    if (f1 > f2) {
        std::cout << f1.toString() << " > " << f2.toString() << std::endl;
    }
    
    // Conversion
    std::cout << "As decimal: " << f1.toDouble() << std::endl;
    
    return 0;
}
```

## Build & Test

```bash
g++ -fdiagnostics-color=always -g fractionTests.cpp -o fractionTests
./fractionTests
```

---

I created the Fraction class myself; test cases generated using Claude Sonnet 4.6. All 115 test cases passed.
