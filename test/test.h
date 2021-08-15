
#pragma once

#include "hexlit.h"

#include <boost/ut.hpp>

#include <foreign/util.h>

#include <iomanip>

namespace foreign::test {
    namespace ut = boost::ut;
    namespace cfg {

        // Mmmm, copypasta...

        class printer {
            [[nodiscard]] inline auto color(const bool cond) {
                const std::string_view colors[] = {colors_.fail, colors_.pass};
                return colors[cond];
            }

        public:
            printer() = default;
            /*explicit(false)*/ printer(const ut::colors colors) : colors_{colors} {}

            template <class T>
            auto& operator<<(const T& t) {
                out_ << ut::detail::get(t);
                return *this;
            }

            template <class T,
                    ut::type_traits::requires_t<ut::type_traits::is_container_v<T> and
            not ut::type_traits::has_npos_v<T>> = 0>
            auto& operator<<(T&& t) {
                *this << '{';
                auto first = true;
                for (const auto& arg : t) {
                    *this << (first ? "" : ", ") << arg;
                    first = false;
                }
                *this << '}';
                return *this;
            }

            template<std::size_t N>
            auto& operator<<(std::array<std::uint8_t, N> buffer) {
                std::stringstream ss;
                ss << std::hex;

                for( int i(0) ; i < N; ++i )
                    ss << std::setw(2) << std::setfill('0') << (int)buffer[i];

                ss << "_hex";

                *this << ss.str();
                return *this;
            }

            auto& operator<<(ut::utility::string_view sv) {
                out_ << sv;
                return *this;
            }

            template <class TLhs, class TRhs>
            auto& operator<<(const ut::detail::eq_<TLhs, TRhs>& op) {
                return (*this << color(op) << op.lhs() << " == " << op.rhs()
                              << colors_.none);
            }

            template <class TLhs, class TRhs>
            auto& operator<<(const ut::detail::neq_<TLhs, TRhs>& op) {
                return (*this << color(op) << op.lhs() << " != " << op.rhs()
                              << colors_.none);
            }

            template <class TLhs, class TRhs>
            auto& operator<<(const ut::detail::gt_<TLhs, TRhs>& op) {
                return (*this << color(op) << op.lhs() << " > " << op.rhs()
                              << colors_.none);
            }

            template <class TLhs, class TRhs>
            auto& operator<<(const ut::detail::ge_<TLhs, TRhs>& op) {
                return (*this << color(op) << op.lhs() << " >= " << op.rhs()
                              << colors_.none);
            }

            template <class TLhs, class TRhs>
            auto& operator<<(const ut::detail::lt_<TRhs, TLhs>& op) {
                return (*this << color(op) << op.lhs() << " < " << op.rhs()
                              << colors_.none);
            }

            template <class TLhs, class TRhs>
            auto& operator<<(const ut::detail::le_<TRhs, TLhs>& op) {
                return (*this << color(op) << op.lhs() << " <= " << op.rhs()
                              << colors_.none);
            }

            template <class TLhs, class TRhs>
            auto& operator<<(const ut::detail::and_<TLhs, TRhs>& op) {
                return (*this << '(' << op.lhs() << color(op) << " and " << colors_.none
                              << op.rhs() << ')');
            }

            template <class TLhs, class TRhs>
            auto& operator<<(const ut::detail::or_<TLhs, TRhs>& op) {
                return (*this << '(' << op.lhs() << color(op) << " or " << colors_.none
                              << op.rhs() << ')');
            }

            template <class T>
            auto& operator<<(const ut::detail::not_<T>& op) {
                return (*this << color(op) << "not " << op.value() << colors_.none);
            }

            template <class T>
            auto& operator<<(const ut::detail::fatal_<T>& fatal) {
                return (*this << fatal.get());
            }

#if defined(__cpp_exceptions)
            template <class TExpr, class TException>
            auto& operator<<(const ut::detail::throws_<TExpr, TException>& op) {
                return (*this << color(op) << "throws<"
                              << std::string_view{ut::reflection::type_name<TException>()}
                              << ">" << colors_.none);
            }

            template <class TExpr>
            auto& operator<<(const ut::detail::throws_<TExpr, void>& op) {
                return (*this << color(op) << "throws" << colors_.none);
            }

            template <class TExpr>
            auto& operator<<(const ut::detail::nothrow_<TExpr>& op) {
                return (*this << color(op) << "nothrow" << colors_.none);
            }
#endif

#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>) and not defined(BOOST_UT_FORWARD)
            template <class TExpr>
            auto& operator<<(const ut::detail::aborts_<TExpr>& op) {
                return (*this << color(op) << "aborts" << colors_.none);
            }
#endif

            template <class T>
            auto& operator<<(const ut::detail::type_<T>&) {
                return (*this << std::string_view{ut::reflection::type_name<T>()});
            }

#if defined(BOOST_UT_FORWARD) or defined(BOOST_UT_IMPLEMENTATION)
            template <class TExpr>
    auto& operator<<(utility::function<TExpr>& expr) {
      void(expr(reinterpret_cast<io::ostream&>(out_)));
      return *this;
    }
#endif
            auto str() const { return out_.str(); }
            const auto& colors() const { return colors_; }

        private:
            ut::colors colors_{};
            std::stringstream out_{};
        };
    }  // namespace cfg

    template<typename TargetType>
    auto umat(TargetType&& target_type)
    {
        return foreign::target_unmaterialize(std::forward<TargetType>(target_type));
    }
}

template <>
auto foreign::test::ut::cfg<foreign::test::ut::override> = ut::runner<ut::reporter<foreign::test::cfg::printer>>{};

