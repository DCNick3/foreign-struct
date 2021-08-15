
add_executable(foreign-ut
        main.cpp
        integer.cpp)

target_link_libraries(foreign-ut ut foreign_struct cxx-sized-int-literals)


