#ifndef TEST_HPP
#define TEST_HPP

#include <iostream>
#include <sstream>

// -- 
// for unit testing, like a very simple version of catch2
// --

#ifndef TEST_COLOUR_MODE
#define TEST_COLOUR_MODE 1 /* enable colour printing where possible */
#endif

// expand macros before stringification
#define __DETAILS_STRINGIFY2(...) #__VA_ARGS__
#define __DETAILS_STRINGIFY1(...) __DETAILS_STRINGIFY2(__VA_ARGS__)
#define __DETAILS_STRINGIFY(...) __DETAILS_STRINGIFY1(__VA_ARGS__)

#if defined __GNUC__ && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#define OVERRIDE override
#elif __cplusplus >= 201103L
#define OVERRIDE override
#else
#define OVERRIDE
#endif

#if defined(_WIN32) || defined(WIN32)
#define __TEST_PATH_SEPARATOR '\\'
#else
#define __TEST_PATH_SEPARATOR '/'
#endif

#define __TEST_GET_CALLER_DETAILS std::move(::__test_details::CallerDetails( \
        __LINE__, \
        ::__test_details::CallerDetails::find_last_occurence_of(__FILE__, __FILE__ + sizeof(__FILE__), __TEST_PATH_SEPARATOR) \
    ))

// generate an evalutated string from a binary/unary expression
// ie if f(x) = x then EVAL(f(2) == f(1)) --> "1 == 2"
#define TEST(...) ::__test_details::assert( \
    __TEST_GET_CALLER_DETAILS, \
    __DETAILS_STRINGIFY(__VA_ARGS__), \
    ::__test_details::CaptureExpr() << __VA_ARGS__ )
    // ie CaptureExpr << lhs [operator] rhs
    // -> UnaryExpr<decltype(lhs)> [operator] rhs
    // -> BinaryExpr<decltype(lhs), decltype(rhs)>

namespace __test_details {
    enum class ANSICode {
        STYLE_RESET = 0,
        STYLE_BOLD = 1,
        STYLE_BOLD_OFF = 21,
        STYLE_DIM = 2,
        STYLE_DIM_OFF = 22,
        STYLE_ITALIC = 3,
        STYLE_UNDERLINE = 4,
        STYLE_UNDERLINE_OFF = 24,
        STYLE_BLINK1 = 5,
        STYLE_BLINK2 = 6,
        STYLE_BLINK_OFF = 25,
        STYLE_REVERSED = 7,
        STYLE_REVERSED_OFF = 27,
        STYLE_CONCEAL = 8,
        STYLE_CONCEAL_OFF = 28,
        STYLE_CROSSED = 9,
        STYLE_CROSSED_OFF = 29,

        FGND_BLACK = 30,
        FGND_RED = 31,
        FGND_GREEN = 32,
        FGND_YELLOW = 33,
        FGND_BLUE = 34,
        FGND_MAGENTA = 35,
        FGND_CYAN = 36,
        FGND_WHITE = 37,

        BGND_BLACK = 40,
        BGND_RED = 41,
        BGND_GREEN = 42,
        BGND_YELLOW = 43,
        BGND_BLUE = 44,
        BGND_MAGENTA = 45,
        BGND_CYAN = 46,
        BGND_WHITE = 47,
    };

    struct ANSI {
        bool m_colour_support;
        ANSICode m_ansi_code;

        explicit ANSI(const ANSICode c, bool force_support=false) 
            : m_ansi_code(c) {
            m_colour_support = force_support; 

            if (force_support)
                return;

            // not so performant since the test for colout support
            // only really needs to happen once.
#if TEST_COLOUR_MODE == 1
            if (auto const env = std::getenv("TERM")) {
                for (auto const& it : {
                    "xterm", "xterm-256", "xterm-256color", "vt100", "color", "ansi", "cygwin", "linux"
                }) {
                    if (std::string(env) == it)
                        m_colour_support = true;
                }
            }
#endif
        }

        friend std::ostream& operator<<(std::ostream& o, const ANSI c) {
            if (c.m_colour_support) {
                o << "\033[";
                o << static_cast<int>(c.m_ansi_code);
                o << "m";
            }

            return o;
        }
    };

    struct CallerDetails {
        const size_t m_line;
        const char* m_file;

        // for example:
        // ~/Documents/img_cmp/src/main.cpp
        // ==> main.cpp
        // sizeof only works for literal
        static constexpr const char* find_last_occurence_of(const char* stop_point, const char* pt, const char& target) {
            return (pt == stop_point || *(pt - 1) == target) ?
                pt : find_last_occurence_of(stop_point, pt - 1, target);
        }

        explicit CallerDetails(const size_t& line, const char* file)
            : m_line(line), m_file(file) { }

        friend std::ostream& operator<<(std::ostream& o, const CallerDetails&& details) {
            o << ANSI(ANSICode::FGND_BLUE);
            o << ANSI(ANSICode::STYLE_BOLD);
            o << details.m_file;
            o << ':';
            o << details.m_line;
            o << ANSI(ANSICode::STYLE_RESET);

            return o;
        }
    };

    struct TokExpr { 
        virtual void write_string(std::ostream& output) const=0;
        virtual bool eval() const=0;
        virtual ~TokExpr() { }

        
        inline friend std::ostream& operator<<(std::ostream& o, const TokExpr& t) {
            t.write_string(o);

            return o;
        }
    };

    template <typename T> struct TrueType : public std::true_type { };
    template <typename T> struct FalseType : public std::false_type { };

    template <typename L, typename R>
    class BinaryExpr : public TokExpr {
        const char* m_op;
        const bool m_value;

        L m_l;
        R m_r;

    public:
        explicit BinaryExpr(L const& l, const char* op, R const& r, bool value) 
            : m_l(l), m_r(r), m_op(op), m_value(value) { }

        void write_string(std::ostream& output) const OVERRIDE {
            output << m_l << ' ' << m_op << ' ' << m_r;
        }

        bool eval() const OVERRIDE{
            return m_value;
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
    class UnaryExpr : public TokExpr { 
        L m_l;

    public:
        explicit UnaryExpr(L const& l)
            : m_l(l) { }

        void write_string(std::ostream& output) const OVERRIDE {
            output << m_l;
        }

        bool eval() const OVERRIDE {
            return static_cast<bool>(m_l);
        }

        // when acted upon by a relational operator
        // a unary expr is converted to a binary expr
        template<typename R>
        auto operator==(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "==", r, m_l == r); }
        template<typename R>
        auto operator!=(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "!=", r, m_l != r); }
        template<typename R>
        auto operator>=(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, ">=", r, m_l >= r); }
        template<typename R>
        auto operator<=(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "<=", r, m_l <= r); }
        template<typename R>
        auto operator>(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, ">", r, m_l > r); }
        template<typename R>
        auto operator<(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "<", r, m_l < r); }

        template<typename R>
        auto operator&&(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "&&", r, m_l && r); }
        template<typename R>
        auto operator||(R const& r) -> BinaryExpr<L, R> { return BinaryExpr<L, R>(m_l, "||", r, m_l || r); }

        // all other operators are implicitly private
    };

    struct CaptureExpr {
        template<typename T>
        auto operator<<(T const& lhs) -> UnaryExpr<T> {
            return UnaryExpr<T>(lhs);
        }
    };
    
    void assert(const CallerDetails&& c, const char* expr_string, TokExpr&& expr_expansion) {
        if (!expr_expansion.eval()) {
            std::cout << std::boolalpha;

            std::cout << ANSI(ANSICode::FGND_RED) << "TEST FAILED" << ANSI(ANSICode::STYLE_RESET);
            std::cout << " (" << std::move(c) << ")\n";
            std::cout << "    GOT: " << expr_string << '\n';
            std::cout << "    WAS: " << expr_expansion << std::endl;
        }
    }
};

#endif // TEST_HPP