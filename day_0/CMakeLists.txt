cmake_minimum_required(VERSION 3.15)
set(DAY_NAME day_0)

project(${DAY_NAME})
set(CMAKE_CXX_STANDARD 23)
add_executable(${DAY_NAME} ${DAY_NAME}.cpp)
