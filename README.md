![workflow badge](https://github.com/DCNick3/foreign-struct/actions/workflows/cmake.yml/badge.svg)

# foreign-struct (name subject to change)

C++20 header-only library to interoperate with C objects of any layout, all in standard C++.

No macros, no (enforced) code generation, no external dependencies, widely extensible.

This project was born as an effort to implement Win32 APIs in a way that would work with any compiler on any platform, not necessarily layout-compatible. It expects the compiler to support used integer sizes though.

## concepts

In general, it works by converting the "target" data representation to "host" (materialization) and back (unmaterialization).
This may seem inefficient, but compilers are smart enough to optimize "materialize, modify, unmaterialize" chains of operations into in-place modification.

Out-of-the box it supports the following object kinds:
- integers
- enums
- structures
- unions
- arrays

While integers, enums and arrays do not require any prior definitions, you would have to supply some info for structures and unions.

### structure definition

For struct you would need to specify the list of fields, their offsets and overall size of the structure. Currently there is no way to automatically lay out the fields.

```C++
using def = foreign::target_struct_def;
using field = foreign::target_struct_field;

using test_struct_def = def<
  // specify the structure size
  16,
  // specify the type and offset of fields
  field<std::uint64_t, 0>,
  field<std::int32_t, 8>
>;
```

The structure can have gaps in it (i.e. regions that are not covered by any fields). This is usually needed to support structure alignment.


After having the structure definition you two options for accessing your members: you either access them via indeces like an `std::tuple`:
```C++
using test_struct = target_struct_unnamed<test_struct_def>;
```

or define a wrapper class which assigns names to the fields:

```C++
struct test_struct : public target_struct_base<test_struct_def, test_struct> {
    inline test_struct(mat_tuple &&values)
            :
            value1(std::move(std::get<0>(values))),
            value2(std::move(std::get<1>(values))),
            value3(std::move(std::get<2>(values))),
            value4(std::move(std::get<3>(values))) {}

    int32_t value1;
    uint64_t value2;
    test_enum value3;
    test_union value4;

    using field_accessor = target_struct_base::field_accessor_base<
            &test_struct::value1,
            &test_struct::value2,
            &test_struct::value3,
            &test_struct::value4
    >;
};
```

Notice the `field_accessor` nested type that allows the library internals to access the fields for unmaterialization. (TODO: do we need the constructor, or can we get away with field_accesor?)

As one can see, this requires quite a lot of boilerplate, so it might be a good idea to generate this code from existing C API definitions

## Extensibility

Can be extended to support different endianness, different types representation, custom pointers, etc (TODO: how?)

TODO: write more examples, mention `target_rw_mat_holder` (name subject to change), exaplain how any why unions are to be used, work out the ideomatic use-cases
