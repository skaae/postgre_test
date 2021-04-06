cmake_minimum_required(VERSION 3.4...3.18)
project(EPEX LANGUAGES CXX C)

# Add executable called "helloDemo" that is built from the source files
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found.
add_executable (testpost main.cpp)
target_compile_features(testpost PUBLIC cxx_std_17)
set_property(TARGET testpost PROPERTY C_STANDARD 17)
# Link the executable to the Hello library. Since the Hello library has
# public include directories we will use those link directories when building
# helloDemo

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_include_directories(testpost PUBLIC
        ${CMAKE_SOURCE_DIR}/thirdparty/taopq/include
        ${CMAKE_SOURCE_DIR}/thirdparty/pq/include
        /usr/include/postgresql
        )
target_link_libraries(testpost
        /usr/lib/x86_64-linux-gnu/libpq.so
        ${CMAKE_SOURCE_DIR}/thirdparty/taopq/lib/libtaopq.a
        Threads::Threads
        )

