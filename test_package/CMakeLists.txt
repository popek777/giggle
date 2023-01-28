cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(giggle CONFIG REQUIRED)

add_executable(chat_gpt src/chat_gpt.cpp)
target_link_libraries(chat_gpt giggle::giggle)
