find_package(PkgConfig)
pkg_check_modules(JANSSON REQUIRED jansson)

#include_directories("${PROJECT_SOURCE_DIR}/inc")
#include_directories("${PROJECT_SOURCE_DIR}/sock_wrapper")
#include_directories("${JANSSON_INCLUDE_DIRS}")
link_directories("${JANSSON_LIBRARY_DIRS}")

add_executable(actor_critic_demo actor_critic_demo.cpp)
add_executable(actor_critic_wall_avoidance_demo actor_critic_wall_avoidance_demo.cpp)
add_executable(simple_control_demo simple_control_demo.cpp)
add_executable(publish_example publish_example.cpp)

add_library(rosmsg_tool
SHARED
	rosmsg_tool.cpp
)


target_include_directories(actor_critic_wall_avoidance_demo PUBLIC "${PROJECT_SOURCE_DIR}/inc")
#target_include_directories(actor_criticwall_avoidance__demo PUBLIC "${PROJECT_SOURCE_DIR}/sock_wrapper")
target_include_directories(actor_critic_wall_avoidance_demo PUBLIC "${JANSSON_INCLUDE_DIRS}")

target_include_directories(actor_critic_demo PUBLIC "${PROJECT_SOURCE_DIR}/inc")
#target_include_directories(actor_critic_demo PUBLIC "${PROJECT_SOURCE_DIR}/sock_wrapper")
target_include_directories(actor_critic_demo PUBLIC "${JANSSON_INCLUDE_DIRS}")

target_include_directories(simple_control_demo PUBLIC "${PROJECT_SOURCE_DIR}/inc")
target_include_directories(simple_control_demo PUBLIC "${JANSSON_INCLUDE_DIRS}")

target_include_directories(publish_example PUBLIC "${PROJECT_SOURCE_DIR}/inc")
#target_include_directories(publish_example PUBLIC "${PROJECT_SOURCE_DIR}/sock_wrapper")
target_include_directories(publish_example PUBLIC "${JANSSON_INCLUDE_DIRS}")


target_include_directories(rosmsg_tool PUBLIC "${JANSSON_INCLUDE_DIRS}")


#target_link_libraries(wspp_wrapper
#${WEBSOCKETPP_LIBRARIES}
#boost_system
#)

target_link_libraries(rosmsg_tool
${JANSSON_LIBRARIES}
)

target_link_libraries(simple_control_demo
${JANSSON_LIBRARIES}
wspp_wrapper
rosmsg_tool
)

target_link_libraries(actor_critic_wall_avoidance_demo
${JANSSON_LIBRARIES}
wspp_wrapper
rosmsg_tool
)
target_link_libraries(actor_critic_demo
${JANSSON_LIBRARIES}
wspp_wrapper
rosmsg_tool
)

target_link_libraries(publish_example
${JANSSON_LIBRARIES}
wspp_wrapper
rosmsg_tool
)
#add_dependencies(stdio_crb_debug crb_client)
