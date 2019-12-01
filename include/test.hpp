#ifndef TEST_HPP
#define TEST_HPP

#include <iostream>
#include <sstream>

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
    __test_details::Stringable::output_stringable(__test_details::CaptureExpr() << __VA_ARGS__))
    // ie CaptureExpr << lhs [operator] rhs
    // -> UnaryExpr<decltype(lhs)> [operator] rhs
    // -> BinaryExpr<decltype(lhs), decltype(rhs)>

namespace __test_details {
    struct None { };

    struct Stringable { 
        virtual void write_string(std::ostream& output)=0;

        static std::string output_stringable(Stringable&& s) {
            std::ostringstream output_ss;

            output_ss << std::boolalpha;

            s.write_string(output_ss);

            return output_ss.str();
        }
    };

    template <typename T> struct TrueType : public std::true_type { };
    template <typename T> struct FalseType : public std::false_type { };

    template <typename L, typename R>
    class BinaryExpr : public Stringable {
        const char* m_op;
        L m_l;
        R m_r;

    public:
        BinaryExpr(L l, const char* op, R r) 
            : m_l(l), m_r(r), m_op(op) { }

        /* override */ void write_string(std::ostream& output) {
            output << m_l << ' ' << m_op << ' ' << m_r;
        }

#define __DETAILS_TEST_BINEXPROPERRMSG \
    "attempt to use relational operator on binary expression to form a trinarary expression" \
    "failed because only unary and binary expressions are supported by the test system i.e." \
    "all expressions must take the form (a) [op] (b)"

        template <typename T>
        void operator==(T const& t) { static_assert(FalseType<T>::value, __DETAILS_TEST_BINEXPROPERRMSG); }
        template <typename T>
        void operator!=(T const& t) { static_assert(FalseType<T>::value, __DETAILS_TEST_BINEXPROPERRMSG); }
        template <typename T>
        void operator>=(T const& t) { static_assert(FalseType<T>::value, __DETAILS_TEST_BINEXPROPERRMSG); }
        template <typename T>
        void operator<=(T const& t) { static_assert(FalseType<T>::value, __DETAILS_TEST_BINEXPROPERRMSG); }
        template <typename T>
        void operator>(T const& t) { static_assert(FalseType<T>::value, __DETAILS_TEST_BINEXPROPERRMSG); }
        template <typename T>
        void operator<(T const& t) { static_assert(FalseType<T>::value, __DETAILS_TEST_BINEXPROPERRMSG); }
        template <typename T>
        void operator&&(T const& t) { static_assert(FalseType<T>::value, __DETAILS_TEST_BINEXPROPERRMSG); }
        template <typename T>
        void operator||(T const& t) { static_assert(FalseType<T>::value, __DETAILS_TEST_BINEXPROPERRMSG); }
    };

    template <typename L>
    class UnaryExpr : public Stringable { 
        L m_l;

    public:
        UnaryExpr(L l)
            : m_l(l) { }

        /* override */ void write_string(std::ostream& output) {
            output << m_l;
        }

        // when acted upon by a relational operator
        // a unary expr is converted to a binary expr
        template<typename R>
        auto operator==(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "==", r); }
        template<typename R>
        auto operator!=(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "!=", r); }
        template<typename R>
        auto operator>=(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, ">=", r); }
        template<typename R>
        auto operator<=(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "<=", r); }
        template<typename R>
        auto operator>(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, ">", r); }
        template<typename R>
        auto operator<(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "<", r); }

        template<typename R>
        auto operator&&(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "&&", r); }
        template<typename R>
        auto operator||(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "||", r); }

        // all other operators are implicitly private
    };

    struct CaptureExpr {
        template<typename T>
        auto operator<<(T const& lhs) -> UnaryExpr<T> {
            return UnaryExpr<T>(lhs);
        }
    };

    std::string eval(const char* expr_string, std::string expr_expansion) {
        return std::string(expr_string) + " [-->] " + expr_expansion;
    }
};

#endif // TEST_HPP