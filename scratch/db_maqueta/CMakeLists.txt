
cmake_minimum_required(VERSION 3.29)

project(db_maqueta)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-write-strings")

#   add_subdirectory(include)

include_directories(include)

add_executable(db_maqueta main.cpp datatable.cpp user.cpp users.cpp database.cpp)
