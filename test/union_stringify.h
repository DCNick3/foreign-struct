#pragma once

#include "foreign/util.h"
#include "foreign/union.h"

#include <sstream>

template<typename ...Members>
struct stringifier<foreign::target_union<Members...>> {
    static auto stringify_value(const foreign::target_union<Members...>& val) {
        //std::stringstream ss;

        return "[union]";
    }
};