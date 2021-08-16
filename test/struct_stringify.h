#pragma once

#include "foreign/util.h"
#include "foreign/struct.h"

#include <sstream>

template<foreign::TargetStruct T>
struct stringifier<T> {
    static auto stringify_value(const T& val) {
        std::stringstream ss;
        ss << '{';
        auto first = true;
        foreign::util::constexpr_for<0, T::def::field_count, 1>([&](auto i) {
            ss << (first ? "" : ", ") << stringify(val.template get_field<i>());
            first = false;
        });
        ss << '}';
        return ss.str();
    }
};