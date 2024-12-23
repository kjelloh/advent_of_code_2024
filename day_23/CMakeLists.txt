cmake_minimum_required(VERSION 3.15)

# Read the day number from the local day.txt file in the same folder as CMakeLists.txt
file(READ "${CMAKE_SOURCE_DIR}/day.txt" DAY_NUMBER)

# Strip out any extraneous CR or LF characters (carriage return or newline)
string(REGEX REPLACE "[\r\n]+" "" DAY_NUMBER "${DAY_NUMBER}")

# Set the day name dynamically based on the day number
set(DAY_NAME "day_${DAY_NUMBER}")

project(${DAY_NAME})
set(CMAKE_CXX_STANDARD 23)

# Set flags for Debug and Release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto")
set(CMAKE_CXX_FLAGS_DEBUG "-O2 -g")

# Create the executable target
add_executable(${DAY_NAME} ${DAY_NAME}.cpp aoc.hpp)

# Now set the Xcode scheme property for the target
if (CMAKE_GENERATOR STREQUAL "Xcode")
    set_property(TARGET ${DAY_NAME} PROPERTY XCODE_GENERATE_SCHEME TRUE)
endif()

# Configure the binary output folder with a file containing the path to the working dir
add_custom_command(
    TARGET ${DAY_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "${CMAKE_SOURCE_DIR}" > $<TARGET_FILE_DIR:${DAY_NAME}>/working_dir.txt
)
