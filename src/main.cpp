#include "test.hpp"

#include <iostream>

int factorial(int n) {
    if (n < 1) 
        return 1;

    return n * factorial(n - 1);
}

int main(int argc, char* argv[]) {

    // unary expressions
    // std::cout << EVAL(42) << std::endl; // --> 42
    // std::cout << EVAL(12.1) << std::endl; // --> 12.1
    // std::cout << EVAL(factorial(3)) << std::endl; // --> 6

    // binary expressions
    std::cout << EVAL(1 == 2) << std::endl; // --> 1 == 2
    std::cout << EVAL(6 == factorial(3)) << std::endl; // --> 6 == 6
    std::cout << EVAL(factorial(1) == factorial(0)) << std::endl; // --> 1 == 1

    // will no doubt fail
    // EVAL(1 + 1 == factorial(0))
    // ie we aim to support:
    // a op b op c op ...

    return 0;
}