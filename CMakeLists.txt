cmake_minimum_required(VERSION 3.20)


# Example cmake project
project(xraymcexample1 VERSION 0.0.1 DESCRIPTION "xraymc example 1" LANGUAGES CXX)

# Setting c++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch xraymc library
# xraymc is rather lightweight and is designed to be downloaded at 
# configure time by cmake's FetchContent module

# Include CMakes FetchContent module
include(FetchContent)
# Specify where to fint xraymc
FetchContent_Declare(
    libxraymc # name of cmake target (you can chose)
    GIT_REPOSITORY https://github.com/medicalphysics/XRayMClib.git # Git address
    GIT_TAG develop # Git branch name (master for stable version) 
)
# Optional, make xraymc download material database from IAEA for you
set(XRAYMCLIB_EPICS_DOWNLOAD "ON")
# Optional, include option to save png files
set(XRAYMCLIB_USE_LOADPNG "ON")

# Fetching the xraylib library
FetchContent_MakeAvailable(libxraymc)

# Generate a target (executable) for the example
add_executable(xraymcexample1 main.cpp)
# Linking to xraymc
target_link_libraries(xraymcexample1 PRIVATE libxraymc)
# Making cmake copy a binary physics database to target build location
# xraymc is dependent on this database
xraymclib_add_physics_list(xraymcexample1)
