cmake_minimum_required(VERSION 3.16)
project(FastSearch VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Add executable
add_executable(fastsearch
    src/main.cpp
    src/cli_interface.cpp
    src/file_indexer.cpp
)

# Include directories
target_include_directories(fastsearch PRIVATE src)

# Link libraries
target_link_libraries(fastsearch PRIVATE Threads::Threads)

# Compiler-specific options
if(MSVC)
    target_compile_options(fastsearch PRIVATE /W4)
else()
    target_compile_options(fastsearch PRIVATE -Wall -Wextra -O3)
endif()

# Optional Qt GUI (uncomment if you want GUI support)
# find_package(Qt6 COMPONENTS Core Widgets QUIET)
# if(Qt6_FOUND)
#     add_executable(fastsearch-gui
#         src/gui_main.cpp
#         src/file_indexer.cpp
#         src/search_window.cpp
#     )
#     target_include_directories(fastsearch-gui PRIVATE src)
#     target_link_libraries(fastsearch-gui PRIVATE Qt6::Core Qt6::Widgets Threads::Threads)
# endif()

# Install target
install(TARGETS fastsearch DESTINATION bin)