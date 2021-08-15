
#pragma once

#include "util.h"

namespace foreign {
    // enum types materialization
    template<Enum T>
    struct materializer<T> {
        using underlying = std::underlying_type_t<T>;

        static auto materialize(span_for_c<T> data) {
            return static_cast<T>(
                    target_materialize<underlying>(
                            data
                    )
            );
        }

        static void unmaterialize(span_for<T> data, T v) {
            target_unmaterialize<underlying>(
                    data,
                    static_cast<underlying>(
                            v
                    )
            );
        }
    };

    template<Enum T>
    struct target_sizeof<T>
            : target_sizeof<std::underlying_type_t<T>> {
    };
}
