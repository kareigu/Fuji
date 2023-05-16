cmake_minimum_required(VERSION 3.15)
project(fuji-sandbox CXX)

find_package(fuji CONFIG REQUIRED)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)


add_executable(fuji-sandbox src/fuji-sandbox.cpp src/main.cpp)

target_link_libraries(fuji-sandbox PRIVATE fuji::fuji)


install(TARGETS fuji-sandbox DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )
