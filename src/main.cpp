#include "test.hpp"

#include <iostream>

int factorial(int n) {
    if (n < 1) 
        return 1;

    return n * factorial(n - 1);
}

inline int square(int x) {
    return x*x;
}

struct Unprintable {
};
int main(int argc, char* argv[]) { 
    // unary expressions
    TEST(42); // --> 42
    TEST(12.1); // --> 12.1
    TEST(factorial(3)); // --> 6

    // binary expressions
    TEST(1 != 2); // --> 1 == 2
    TEST(6 != factorial(3)); // --> 6 == 6
    TEST(factorial(1) != factorial(0)); // --> 1 == 1

    TEST((true || false) == true); // --> true == true
    TEST(1 + 1 == factorial(0));

    // check that single call is maintained
    TEST(square(2));
    TEST(square(1) == 1);
    TEST(9 == square(3));
    TEST(9 + 1 == 1 + square(3));

    // push limits of printability
    TEST(Unprintable());

    std::cout << __test_details::CheckPrintability<int>::value << std::endl;
    std::cout << __test_details::CheckPrintability<Unprintable>::value << std::endl;

    return 0;
}