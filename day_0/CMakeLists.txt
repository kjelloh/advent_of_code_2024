cmake_minimum_required(VERSION 3.15)

# Read the day number from the local day.txt file in the same folder as CMakeLists.txt
file(READ "${CMAKE_SOURCE_DIR}/day.txt" DAY_NUMBER)

# Strip out any extraneous CR or LF characters (carriage return or newline)
string(REGEX REPLACE "[\r\n]+" "" DAY_NUMBER "${DAY_NUMBER}")

# Set the day name dynamically based on the day number
set(DAY_NAME "day_${DAY_NUMBER}")

project(${DAY_NAME})
set(CMAKE_CXX_STANDARD 23)
add_executable(${DAY_NAME} ${DAY_NAME}.cpp)

if (CMAKE_GENERATOR STREQUAL "Xcode")
    # Generate a scheme for the target to specify what binary to run
    set_property(TARGET ${DAY_NAME} PROPERTY XCODE_GENERATE_SCHEME TRUE)
endif()
