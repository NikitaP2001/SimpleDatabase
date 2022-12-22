#
# Dependencies
#
find_package(Threads)

#
# Sources
#
set(SOURCES
    src/main.cpp
	src/client.cpp
	src/client.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

#
# Target
#
add_executable(client ${SOURCES})
target_include_directories(client PRIVATE ../../)
target_link_libraries(client
    PRIVATE
        myproto
)
