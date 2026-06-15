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
    using C = checked_arith_detail::CheckedInt<int>;

    std::cout << static_cast<int> (checked_arith_detail::floor_div(C(-1), C(3))) << std::endl;
    
    return 0;
}