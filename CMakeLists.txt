cmake_minimum_required(VERSION 3.15)
#cmake_minimum_required(VERSION 2.8)
project(crosbridge CXX)
set(CMAKE_CXX_FLAGS_DEBUG "-g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(PkgConfig)
pkg_check_modules(JANSSON REQUIRED jansson)

add_subdirectory("${PROJECT_SOURCE_DIR}/demo_src")
add_subdirectory("${PROJECT_SOURCE_DIR}/src")

include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${JANSSON_INCLUDE_DIRS}")
link_directories("${JANSSON_LIBRARY_DIRS}")

add_library(cpp_rosbridge INTERFACE)
target_link_libraries(cpp_rosbridge 
	INTERFACE wspp_wrapper
)