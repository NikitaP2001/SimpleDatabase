#
# Dependencies
#
find_package(Threads)
#
# Sources
#

set(SOURCES
    src/main.cpp
	src/server.cpp
	src/server.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

file(GLOB DBSRC
  "${PROJECT_SOURCE_DIR}../../*.cpp"
  "${PROJECT_SOURCE_DIR}../../*.cxx"
)
add_library(db OBJECT ${DBSRC})
#
# Target
#
add_executable(server ${SOURCES})
target_include_directories(server PRIVATE ../../)
target_link_libraries(server
    PRIVATE
        myproto
		db
)
