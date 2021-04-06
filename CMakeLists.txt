cmake_minimum_required(VERSION 2.8.2)
set(CMAKE_CXX_STANDARD 17)
set( CMAKE_VERBOSE_MAKEFILE on )
set(CMAKE_CXX_FLAGS "-Wall -std=gnu++17 -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Add executable called "helloDemo" that is built from the source files
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found.
add_executable (testpost main.cpp)
target_compile_features(testpost PUBLIC cxx_std_17)
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

