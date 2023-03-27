#find_package(websocketpp)
find_package(Boost)

add_library(wspp_wrapper
SHARED
	wspp_wrapper.cpp
)

target_link_libraries(wspp_wrapper
${WEBSOCKETPP_LIBRARIES}
boost_system
pthread
)

target_include_directories(wspp_wrapper
PUBLIC ${PROJECT_SOURCE_DIR}/inc
PUBLIC ${Boost_INCLUDE_DIRS}
PUBLIC ${WEBSOCKETPP_INCLUDE_DIR}
)
