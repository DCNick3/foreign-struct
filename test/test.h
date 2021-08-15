
#pragma once

#include "cli.h"

#include <foreign/util.h>

#include "hexlit.h"

#include <boost/ut.hpp>

#include <iomanip>
#include <set>
#include <optional>
#include <concepts>
#include <functional>

namespace foreign::test {
    namespace ut = boost::ut;

    template<typename TargetType>
    struct umat_chk {
        using buffer_t = array_for<TargetType>;

        TargetType _value;
        buffer_t _expected_unmaterialization, _actual_unmaterialization;

        umat_chk(TargetType &&value, buffer_t expected_unmaterialization)
                : _value(std::forward<TargetType>(value)), _expected_unmaterialization(expected_unmaterialization),
                  _actual_unmaterialization(target_unmaterialize(_value)) {}

        operator bool() {
            return _expected_unmaterialization == _actual_unmaterialization;
        }
    };

    template<typename TargetType>
    struct mat_chk {
        using buffer_t = array_for<TargetType>;

        buffer_t _unmaterialized;
        TargetType _expected_value, _actual_value;

        mat_chk(buffer_t unmaterialized, TargetType &&expected_value)
                : _unmaterialized(unmaterialized), _expected_value(std::forward<TargetType>(expected_value)),
                  _actual_value(target_materialize<TargetType>(_unmaterialized)) {}

        operator bool() {
            return _expected_value == _actual_value;
        }
    };

    template<typename TargetType>
    struct acc_chk {
        using buffer_t = array_for<TargetType>;

        buffer_t _initial, _expected_result, _actual_result;

        acc_chk(buffer_t initial, std::function<void (TargetType&)> func, buffer_t expected_result)
            : _initial(initial)
            , _expected_result(expected_result)
            , _actual_result([&]() {
                    buffer_t buf(_initial);
                    {
                        target_rw_mat_holder<TargetType> holder(buf);
                        func(*holder);
                    }
                    return buf;
            }())
        {}

        operator bool() {
            return _expected_result == _actual_result;
        }
    };

    template<typename T>
    struct stringifier
    {};

    namespace detail {
        template<std::size_t N>
        struct string_literal {
            static constexpr std::size_t length = N;
            std::array<char, N> arr_;

            constexpr string_literal(const char(&in)[N]) : arr_{} {
                std::copy(in, in + N, arr_.begin());
            }
        };

        template<typename T, string_literal suffix>
        struct int_stringifier
        {
            static auto stringify(T t) { return std::to_string(std::forward<T>(t)) + suffix.arr_.data(); }
        };
    }

    template<>
    struct stringifier<std::uint8_t> : public detail::int_stringifier<std::uint8_t, "_u8"> {};
    template<>
    struct stringifier<std::uint16_t> : public detail::int_stringifier<std::uint16_t, "_u16"> {};
    template<>
    struct stringifier<std::uint32_t> : public detail::int_stringifier<std::uint32_t, "_u32"> {};

    template<>
    struct stringifier<std::int8_t> : public detail::int_stringifier<std::int8_t, "_i8"> {};
    template<>
    struct stringifier<std::int16_t> : public detail::int_stringifier<std::int16_t, "_i16"> {};
    template<>
    struct stringifier<std::int32_t> : public detail::int_stringifier<std::int32_t, "_i32"> {};

    template<foreign::Enum E>
    struct stringifier<E> {
        using underlying = std::underlying_type_t<E>;
        static auto stringify(E val) {
            return stringifier<underlying>::stringify(static_cast<underlying>(val));
        }
    };

    template<typename T>
    auto stringify(T&& t) { return stringifier<std::remove_cvref_t<T>>::stringify(std::forward<T>(t)); }

    namespace detail {
        bool is_listing_tests = false;
        std::optional<std::set<std::string>> run_tests_filter;

        template<class TReporter, auto MaxPathSize = 16>
        class runner : public ut::runner<TReporter> {
        private:
            using base = ut::runner<TReporter>;

        public:
            using base::on;

            template<class... Ts>
            auto on(ut::events::test<Ts...> test) {
                if (not is_listing_tests) {
                    if (!run_tests_filter or run_tests_filter->contains(test.name)) {
                        if (run_tests_filter)
                            run_tests_filter->erase(std::string(test.name));
                        base::on(test);
                    }
                } else {
                    std::cout << test.name << '\n';
                }
            }
        };

        template<typename TPrinter>
        class reporter : public ut::reporter<TPrinter> {
        private:
            using base = ut::reporter<TPrinter>;
        public:
            using base::on;

            auto on(ut::events::summary summary) -> void {
                // we don't want to print anything extra when listing tests
                if (not is_listing_tests) {
                    base::on(summary);
                }
            }
        };

        class printer {
        private:
            [[nodiscard]] inline auto color(const bool cond) {
                const std::string_view colors[] = {colors_.fail, colors_.pass};
                return colors[cond];
            }

        public:

            printer() = default;

            /*explicit(false)*/ printer(const ut::colors colors) : colors_{colors} {}

            template<class T>
            auto &operator<<(const T &t) {
                out_ << ut::detail::get(t);
                return *this;
            }

            template<std::size_t N>
            auto &operator<<(std::array<std::uint8_t, N> buffer) {
                std::stringstream ss;
                ss << std::hex;

                for (int i(0); i < N; ++i)
                    ss << std::setw(2) << std::setfill('0') << (int) buffer[i];

                ss << "_hex";

                *this << ss.str();
                return *this;
            }

            template<typename TargetType>
            auto &operator<<(const umat_chk<TargetType> &chk) {
                *this << "umat(" << stringify(chk._value) << ") /* "
                      << chk._actual_unmaterialization << " */ == "
                      << chk._expected_unmaterialization;
                return *this;
            }

            template<typename TargetType>
            auto &operator<<(const mat_chk<TargetType> &chk) {
                *this << "mat(" << chk._unmaterialized << ") /* "
                      << stringify(chk._actual_value) << " */ == "
                      << stringify(chk._expected_value);
                return *this;
            }

            template<typename TargetType>
            auto& operator<<(const acc_chk<TargetType>& chk) {
                *this << chk._actual_result << " == " << chk._expected_result;
                return *this;
            }

            template<class T,
                    ut::type_traits::requires_t<ut::type_traits::is_container_v<T> and
                                                not ut::type_traits::has_npos_v<T>> = 0>
            auto &operator<<(T &&t) {
                *this << '{';
                auto first = true;
                for (const auto &arg : t) {
                    *this << (first ? "" : ", ") << arg;
                    first = false;
                }
                *this << '}';
                return *this;
            }

            auto &operator<<(ut::utility::string_view sv) {
                out_ << sv;
                return *this;
            }

            template<class TLhs, class TRhs>
            auto &operator<<(const ut::detail::eq_<TLhs, TRhs> &op) {
                return (*this << color(op) << op.lhs() << " == " << op.rhs()
                              << colors_.none);
            }

            template<class TLhs, class TRhs>
            auto &operator<<(const ut::detail::neq_<TLhs, TRhs> &op) {
                return (*this << color(op) << op.lhs() << " != " << op.rhs()
                              << colors_.none);
            }

            template<class TLhs, class TRhs>
            auto &operator<<(const ut::detail::gt_<TLhs, TRhs> &op) {
                return (*this << color(op) << op.lhs() << " > " << op.rhs()
                              << colors_.none);
            }

            template<class TLhs, class TRhs>
            auto &operator<<(const ut::detail::ge_<TLhs, TRhs> &op) {
                return (*this << color(op) << op.lhs() << " >= " << op.rhs()
                              << colors_.none);
            }

            template<class TLhs, class TRhs>
            auto &operator<<(const ut::detail::lt_<TRhs, TLhs> &op) {
                return (*this << color(op) << op.lhs() << " < " << op.rhs()
                              << colors_.none);
            }

            template<class TLhs, class TRhs>
            auto &operator<<(const ut::detail::le_<TRhs, TLhs> &op) {
                return (*this << color(op) << op.lhs() << " <= " << op.rhs()
                              << colors_.none);
            }

            template<class TLhs, class TRhs>
            auto &operator<<(const ut::detail::and_<TLhs, TRhs> &op) {
                return (*this << '(' << op.lhs() << color(op) << " and " << colors_.none
                              << op.rhs() << ')');
            }

            template<class TLhs, class TRhs>
            auto &operator<<(const ut::detail::or_<TLhs, TRhs> &op) {
                return (*this << '(' << op.lhs() << color(op) << " or " << colors_.none
                              << op.rhs() << ')');
            }

            template<class T>
            auto &operator<<(const ut::detail::not_<T> &op) {
                return (*this << color(op) << "not " << op.value() << colors_.none);
            }

            template<class T>
            auto &operator<<(const ut::detail::fatal_<T> &fatal) {
                return (*this << fatal.get());
            }

#if defined(__cpp_exceptions)

            template<class TExpr, class TException>
            auto &operator<<(const ut::detail::throws_<TExpr, TException> &op) {
                return (*this << color(op) << "throws<"
                              << std::string_view{ut::reflection::type_name<TException>()}
                              << ">" << colors_.none);
            }

            template<class TExpr>
            auto &operator<<(const ut::detail::throws_<TExpr, void> &op) {
                return (*this << color(op) << "throws" << colors_.none);
            }

            template<class TExpr>
            auto &operator<<(const ut::detail::nothrow_<TExpr> &op) {
                return (*this << color(op) << "nothrow" << colors_.none);
            }

#endif

#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>) and not defined(BOOST_UT_FORWARD)

            template<class TExpr>
            auto &operator<<(const ut::detail::aborts_<TExpr> &op) {
                return (*this << color(op) << "aborts" << colors_.none);
            }

#endif

            template<class T>
            auto &operator<<(const ut::detail::type_<T> &) {
                return (*this << std::string_view{ut::reflection::type_name<T>()});
            }

#if defined(BOOST_UT_FORWARD) or defined(BOOST_UT_IMPLEMENTATION)
            template <class TExpr>
    auto& operator<<(ut::utility::function<TExpr>& expr) {
      void(expr(reinterpret_cast<io::ostream&>(out_)));
      return *this;
    }
#endif

            auto str() const { return out_.str(); }

            const auto &colors() const { return colors_; }

        private:
            ut::colors colors_{};
            std::stringstream out_{};
        };

        using cfg_t = runner<reporter<printer>>;
    }
}
template <>
auto foreign::test::ut::cfg<foreign::test::ut::override> =
        foreign::test::detail::cfg_t{};

namespace foreign::test {
    namespace detail {
        int run_tests(bool is_listing_tests_, std::vector<std::string> run_tests) {
            is_listing_tests = is_listing_tests_;
            if (not run_tests.empty()) {
                run_tests_filter = std::set<std::string>();
                run_tests_filter->insert(run_tests.begin(), run_tests.end());
            }

            const auto has_fails = foreign::test::ut::cfg<>.run(
                    {.report_errors =
                    true});  // explicitly run registered test suites and report errors

            if (run_tests_filter and not run_tests_filter->empty()) {
                std::cerr << "Warning: not all specified tests were ran\nHere's a list of the ones left:\n";
                for (auto t : *run_tests_filter)
                    std::cerr << t << "\n";
                std::cerr.flush();
            }

            return has_fails ? 1 : 0;
        }
    }

    template<typename TargetType>
    auto umat(TargetType&& target_type)
    {
        return foreign::target_unmaterialize(std::forward<TargetType>(target_type));
    }
}

using namespace std::string_literals;
using namespace foreign::test;

