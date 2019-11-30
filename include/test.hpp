#ifndef TEST_HPP
#define TEST_HPP

#include <string>
#include <list>

// -- 
// for unit testing, like a very simple version of catch2
// --

// expand macros before stringification
#define __DETAILS_STRINGIFY2(...) #__VA_ARGS__
#define __DETAILS_STRINGIFY1(...) __DETAILS_STRINGIFY2(__VA_ARGS__)
#define __DETAILS_STRINGIFY(...) __DETAILS_STRINGIFY1(__VA_ARGS__)

// generate an evalutated string from a binary/unary expression
// ie if f(x) = x then EVAL(f(2) == f(1)) --> "1 == 2"
#define EVAL(...) __test_details::eval( \
    __DETAILS_STRINGIFY(__VA_ARGS__), \
    (__test_details::TermCapture() << __VA_ARGS__).get_string())
    // ie TermCapture << lhs [operator] rhs
    // -> LhsTerm<decltype(lhs)> [operator] rhs
    // -> UBExpr<decltype(lhs), decltype(rhs)>

namespace __test_details {
    struct None { };

    template <typename T> struct TrueType : std::true_type { };
    template <typename T> struct FalseType : std::false_type { };

    struct Stringable { 
        virtual std::string get_string()=0;
    };

    template <typename L, typename R>
    class UBExpr : Stringable {
        const char* m_op;
        L m_l;
        R m_r;

    public:
        UBExpr(L l, const char* op, R r) 
            : m_l(l), m_r(r), m_op(op) { }

        std::string get_string() {
            return std::to_string(m_l) + m_op + std::to_string(m_r);
        }

        template <typename T>
        constexpr void operator==(T const& t) { static_assert(TrueType<T>::value, "bad expr"); }
        template <typename T>
        constexpr void operator!=(T const& t) { static_assert(TrueType<T>::value, "bad expr"); }
        template <typename T>
        constexpr void operator>=(T const& t) { static_assert(TrueType<T>::value, "bad expr"); }
        template <typename T>
        constexpr void operator<=(T const& t) { static_assert(TrueType<T>::value, "bad expr"); }
        template <typename T>
        constexpr void operator>(T const& t) { static_assert(TrueType<T>::value, "bad expr"); }
        template <typename T>
        constexpr void operator<(T const& t) { static_assert(TrueType<T>::value, "bad expr"); }
    };

    template <typename L>
    class LhsTerm : Stringable {
        L m_l;

    public:
        LhsTerm(L l) : m_l(l) { }

        std::string get_string() {
            return std::to_string(m_l);
        }

        template<typename R>
        auto operator==(R const& rt) -> UBExpr<L, R> { return UBExpr<L, R>(m_l, "==", rt); }
        template<typename R>
        auto operator!=(R const& rt) -> UBExpr<L, R> { return UBExpr<L, R>(m_l, "!=", rt); }
        template<typename R>
        auto operator>=(R const& rt) -> UBExpr<L, R> { return UBExpr<L, R>(m_l, ">=", rt); }
        template<typename R>
        auto operator<=(R const& rt) -> UBExpr<L, R> { return UBExpr<L, R>(m_l, "<=", rt); }
        template<typename R>
        auto operator>(R const& rt) -> UBExpr<L, R> { return UBExpr<L, R>(m_l, ">", rt); }
        template<typename R>
        auto operator<(R const& rt) -> UBExpr<L, R> { return UBExpr<L, R>(m_l, "<", rt); }
    };

    struct TermCapture {
        template<typename T>
        auto operator<<(T const& lhs) -> LhsTerm<T> {
            return LhsTerm<T>(lhs);
        }
    };

    std::string eval(const char* expr_string, std::string expr_expansion) {
        return std::string(expr_string) + " [-->] " + expr_expansion;
    }
};

#endif // TEST_HPP