cmake_minimum_required(VERSION 3.15)
project(fuji CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(fmt REQUIRED)
find_package(SDL REQUIRED)

add_library(fuji src/fuji.cpp)
target_include_directories(fuji PUBLIC include)


target_link_libraries(fuji PRIVATE 
  fmt::fmt
  SDL::SDL)

set_target_properties(fuji PROPERTIES PUBLIC_HEADER "include/fuji.h")
install(TARGETS fuji)
