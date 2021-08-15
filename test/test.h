
#pragma once

#include "cli.h"

#include <foreign/util.h>

#include "hexlit.h"

#include <boost/ut.hpp>

#include <iomanip>
#include <set>
#include <optional>

namespace foreign::test {
    namespace ut = boost::ut;

    template<typename TargetType>
    struct umat_chk {
        using buffer_t = array_for<TargetType>;

        TargetType _value;
        buffer_t _expected_unmaterialization, _actual_unmaterialization;

        umat_chk(TargetType &&value, buffer_t expected_unmaterialization)
                : _value(value), _expected_unmaterialization(expected_unmaterialization),
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
                : _unmaterialized(unmaterialized), _expected_value(expected_value),
                  _actual_value(target_materialize<TargetType>(_unmaterialized)) {}

        operator bool() {
            return _expected_value == _actual_value;
        }
    };

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
                }
                else
                {
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

        class printer : public ut::printer {
        private:
            using base = ut::printer;

        public:
            using base::operator<<;

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

