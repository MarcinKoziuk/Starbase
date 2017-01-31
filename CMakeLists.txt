cmake_minimum_required(VERSION 2.7)
project(STARBASE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# --- Definitions ---
set(STARBASE_VERSION "0.1-dev")
set(STARBASE_MAJOR_VERSION 0)
set(STARBASE_MINOR_VERSION 1)

# --- Compile options ---
option(STARBASE_STATIC_VARIANT "Build static and minimal variant" OFF)
option(STARBASE_STATIC_DEPENDENCIES "Whether the dependency libraries have been built as static or shared" OFF)
option(STARBASE_COPY_DLLS "Whether to copy DLL files to build directory (only for development) [WINDOWS]" OFF)
option(STARBASE_SYMLINK_DATA "Whether to symlink data dir to build directory (only for development) [WINDOWS]" OFF)

# --- Target names ---
set(STARBASE_GAME_LIBRARY game)
set(STARBASE_CGAME_LIBRARY cgame)
set(STARBASE_SUPPORT_LIBRARY support)
set(STARBASE_SERVER_EXECUTABLE serv)
set(STARBASE_CLIENT_EXECUTABLE client)
set(STARBASE_HEADERS starbase)
set(STARBASE_DATA data)

# --- Project files ---
file(GLOB_RECURSE STARBASE_GAME_H "src/game/*.hpp" "include/starbase/game/*.hpp" "include/starbase/game/*.inl")
file(GLOB_RECURSE STARBASE_GAME_SRC "src/game/*.cpp")

file(GLOB_RECURSE STARBASE_CGAME_H "src/cgame/*.hpp" "include/starbase/cgame/*.hpp")
file(GLOB_RECURSE STARBASE_CGAME_SRC "src/cgame/*.cpp")

file(GLOB_RECURSE STARBASE_SUPPORT_H "src/support/*.hpp" "include/starbase/support/*.hpp")
file(GLOB_RECURSE STARBASE_SUPPORT_SRC "src/support/*.cpp")

file(GLOB_RECURSE STARBASE_SERVER_H "src/server/*.hpp")
file(GLOB_RECURSE STARBASE_SERVER_SRC "src/server/*.cpp")

file(GLOB_RECURSE STARBASE_CLIENT_H "src/client/*.hpp")
file(GLOB_RECURSE STARBASE_CLIENT_SRC "src/client/*.cpp")

file(GLOB STARBASE_H "include/starbase/*.hpp")

file(GLOB_RECURSE STARBASE_DATA_FILES "data/*")

file(GLOB_RECURSE EXTLIBS_GAME_H
	"extlibs/physfs/include/*.hpp"
	"extlibs/nanosvg/nanosvg.h"
)
file(GLOB_RECURSE EXTLIBS_GAME_SRC
	"extlibs/physfs/src/*.cpp"
	"extlibs/nanosvg/nanosvg.c"
)

# --- Targets ---
add_library(${STARBASE_GAME_LIBRARY} STATIC
    ${STARBASE_GAME_SRC}
    ${STARBASE_GAME_H}
	${EXTLIBS_GAME_SRC}
	${EXTLIBS_GAME_H}
)

add_library(${STARBASE_CGAME_LIBRARY} STATIC
    ${STARBASE_CGAME_SRC}
    ${STARBASE_CGAME_H}
)

add_library(${STARBASE_SUPPORT_LIBRARY} STATIC
    ${STARBASE_SUPPORT_SRC}
    ${STARBASE_SUPPORT_H}
)

add_executable(${STARBASE_SERVER_EXECUTABLE}
    ${STARBASE_SERVER_SRC}
    ${STARBASE_SERVER_H}
)

add_executable(${STARBASE_CLIENT_EXECUTABLE}
    ${STARBASE_CLIENT_SRC}
    ${STARBASE_CLIENT_H}
)

add_custom_target(${STARBASE_HEADERS} SOURCES ${STARBASE_H})
add_custom_target(${STARBASE_DATA} SOURCES ${STARBASE_DATA_FILES})

# --- Set up source groups for visual studio
set(ALL_SOURCES
	${STARBASE_GAME_H}
	${STARBASE_GAME_SRC}
	${STARBASE_CGAME_H}
	${STARBASE_CGAME_SRC}
	${STARBASE_SUPPORT_H}
	${STARBASE_SUPPORT_SRC}
	${STARBASE_SERVER_H}
	${STARBASE_SERVER_SRC}
	${STARBASE_CLIENT_H}
	${STARBASE_CLIENT_SRC}
	${STARBASE_H}
	${STARBASE_DATA_FILES}
	${EXTLIBS_GAME_H}
	${EXTLIBS_GAME_SRC}
)

foreach(FILE ${ALL_SOURCES})
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
    string(REPLACE "/" "\\" GROUP "${GROUP}")
    source_group("${GROUP}" FILES "${FILE}")
endforeach()

# --- Compilation options ---
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /MP /fp:fast")
	#add_definitions(/utf-8 /wd4828 /EHsc) ???
	add_definitions(/wd4996)
else()
    set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra -Wstrict-aliasing -pedantic -ffast-math")
endif()

if(STARBASE_STATIC_DEPENDENCIES)
	add_definitions(-DGLEW_STATIC=1)
endif()

add_definitions(-DSTARBASE_VERSION="${STARBASE_VERSION}")
add_definitions(-DSTARBASE_MAJOR_VERSION="${STARBASE_MAJOR_VERSION}")
add_definitions(-DSTARBASE_MINOR_VERSION="${STARBASE_MINOR_VERSION}")
add_definitions(-DSTARBASE_COMPILETIME_SOURCE_PATH="${CMAKE_SOURCE_DIR}")
add_definitions(-DGLM_FORCE_RADIANS=1)
add_definitions(-DBOOST_ALL_NO_LIB=1)

# --- Dependencies ---
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLM REQUIRED)
find_package(Chipmunk REQUIRED)
find_package(PhysFS REQUIRED)
find_package(YamlCpp REQUIRED)
find_package(Freetype REQUIRED)
find_package(TurboBadger REQUIRED)

# --- Include directories ---
include_directories("include")
include_directories("extlibs/physfs/include")
include_directories("extlibs/nanosvg")
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${OPENGL_INCLUDE_DIR})
include_directories(SYSTEM ${GLEW_INCLUDE_DIR})
include_directories(SYSTEM ${SDL2_INCLUDE_DIR})
include_directories(SYSTEM ${GLM_INCLUDE_DIR})
include_directories(SYSTEM ${CHIPMUNK_INCLUDE_DIR})
include_directories(SYSTEM ${PHYSFS_INCLUDE_DIR})
include_directories(SYSTEM ${YAMLCPP_INCLUDE_DIR})
include_directories(SYSTEM ${TURBOBADGER_INCLUDE_DIR} ${TURBOBADGER_INCLUDE_DIR}/tb)

# --- Linking ---
target_link_libraries(${STARBASE_GAME_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${PHYSFS_LIBRARY}
    ${CHIPMUNK_LIBRARY}
    ${STARBASE_SUPPORT_LIBRARY}
)

target_link_libraries(${STARBASE_CGAME_LIBRARY}
    ${OPENGL_LIBRARY}
    ${GLEW_LIBRARY}
	${SDL2_LIBRARY}
	${FREETYPE_LIBRARY}
    ${TURBOBADGER_LIBRARY}
    ${STARBASE_GAME_LIBRARY}
)

target_link_libraries(${STARBASE_SUPPORT_LIBRARY}
    ${YAMLCPP_LIBRARY}
)

target_link_libraries(${STARBASE_CLIENT_EXECUTABLE}
    ${STARBASE_CGAME_LIBRARY}
)

if (WIN32)
	target_link_libraries(${STARBASE_CLIENT_EXECUTABLE}
		${SDL2MAIN_LIBRARY}
		winmm
		imm32
		Version
	)

	if (STARBASE_STATIC_DEPENDENCIES)
		set_target_properties(${STARBASE_CLIENT_EXECUTABLE} PROPERTIES LINK_FLAGS "/FORCE:multiple")
	endif()
endif()

# --- Additional build commands ---
if (STARBASE_COPY_DLLS AND WIN32)
	file(GLOB STARBASE_DEPENDENCY_DLLS
		${CMAKE_PREFIX_PATH}/lib/*.dll
        ${CMAKE_PREFIX_PATH}/bin/*.dll
	)

	foreach(STARBASE_DEPENDENCY_DLL ${STARBASE_DEPENDENCY_DLLS})
		add_custom_command(TARGET ${STARBASE_CLIENT_EXECUTABLE} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${STARBASE_DEPENDENCY_DLL}"
				$<TARGET_FILE_DIR:${STARBASE_CLIENT_EXECUTABLE}>
		)
	endforeach()
endif()

