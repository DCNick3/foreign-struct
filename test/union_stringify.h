#pragma once

#include "foreign/util.h"
#include "foreign/union.h"

#include <sstream>

template<std::size_t Size, typename ...Members>
struct stringifier<foreign::target_union<Size, Members...>> {
    static auto stringify_value(const foreign::target_union<Size, Members...>& val) {
        //std::stringstream ss;

        return "[union]";
    }
};