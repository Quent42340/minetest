add_subdirectory(network)
add_subdirectory(script)
add_subdirectory(unittest)
add_subdirectory(irrlicht_changes)

file(GLOB         legacy_SRCS    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")
file(GLOB_RECURSE algorithm_SRCS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "algorithm/*.cpp")
file(GLOB_RECURSE content_SRCS   RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "content/*.cpp")
file(GLOB_RECURSE core_SRCS      RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "core/*.cpp")
file(GLOB_RECURSE craft_SRCS     RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "craft/*.cpp")
file(GLOB_RECURSE database_SRCS  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "database/*.cpp")
file(GLOB_RECURSE gui_SRCS       RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "gui/*.cpp")
file(GLOB_RECURSE item_SRCS      RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "item/*.cpp")
file(GLOB_RECURSE map_SRCS       RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "map/*.cpp")
file(GLOB_RECURSE mapgen_SRCS    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "mapgen/*.cpp")
file(GLOB_RECURSE player_SRCS    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "player/*.cpp")
file(GLOB_RECURSE server_SRCS    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "server/*.cpp")
file(GLOB_RECURSE threading_SRCS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "threading/*.cpp")
file(GLOB_RECURSE util_SRCS      RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "util/*.cpp")
file(GLOB_RECURSE world_SRCS     RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "world/*.cpp")

list(REMOVE_ITEM legacy_SRCS "settings_translation_file.cpp")
list(REMOVE_ITEM gui_SRCS "gui/touchscreengui.h" "gui/touchscreengui.cpp")
list(APPEND util_SRCS "util/sha256.c")

set(common_SRCS
	${common_SCRIPT_SRCS}
	${common_network_SRCS}
	${legacy_SRCS}
	${algorithm_SRCS}
	${content_SRCS}
	${core_SRCS}
	${craft_SRCS}
	${database_SRCS}
	${item_SRCS}
	${map_SRCS}
	${mapgen_SRCS}
	${player_SRCS}
	${server_SRCS}
	${threading_SRCS}
	${unittest_SRCS}
	${util_SRCS}
	${world_SRCS}
)

# Client sources
if (BUILD_CLIENT)
	add_subdirectory(client)
endif(BUILD_CLIENT)

set(client_SRCS
	${client_SRCS}
	${common_SRCS}
	${gui_SRCS}
	${client_network_SRCS}
	${client_irrlicht_changes_SRCS}
	${client_SCRIPT_SRCS}
	${unittest_CLIENT_SRCS}
)

list(SORT client_SRCS)

# Server sources
set(server_SRCS
	${common_SRCS}
)

list(SORT server_SRCS)
# Avoid source_group on broken CMake version.
# see issue #7074 #7075
if (CMAKE_VERSION VERSION_GREATER 3.8.1)
	source_group(TREE ${PROJECT_SOURCE_DIR} PREFIX "Source Files" FILES ${client_SRCS})
	source_group(TREE ${PROJECT_SOURCE_DIR} PREFIX "Source Files" FILES ${server_SRCS})
endif()

include_directories(
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}
	${IRRLICHT_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${CMAKE_BUILD_TYPE}
	${PNG_INCLUDE_DIR}
	${GETTEXT_INCLUDE_DIR}
	${SOUND_INCLUDE_DIRS}
	${SQLITE3_INCLUDE_DIR}
	${LUA_INCLUDE_DIR}
	${GMP_INCLUDE_DIR}
	${JSON_INCLUDE_DIR}
	${X11_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/script
)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")

# This gives us the icon and file version information
if(WIN32)
	set(WINRESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../misc/winresource.rc")
	set(MINETEST_EXE_MANIFEST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../misc/minetest.exe.manifest")
	if(MINGW)
		if(NOT CMAKE_RC_COMPILER)
			set(CMAKE_RC_COMPILER "windres.exe")
		endif()
		ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/winresource_rc.o
			COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR} -I${CMAKE_CURRENT_BINARY_DIR}
			-i${WINRESOURCE_FILE}
			-o ${CMAKE_CURRENT_BINARY_DIR}/winresource_rc.o
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			DEPENDS ${WINRESOURCE_FILE})
		SET(extra_windows_SRCS ${CMAKE_CURRENT_BINARY_DIR}/winresource_rc.o)
	else(MINGW) # Probably MSVC
		set(extra_windows_SRCS ${WINRESOURCE_FILE} ${MINETEST_EXE_MANIFEST_FILE})
	endif(MINGW)
endif()

if(BUILD_CLIENT)
	add_executable(${PROJECT_NAME} ${client_SRCS} ${extra_windows_SRCS})
	add_dependencies(${PROJECT_NAME} GenerateVersion)
	set(client_LIBS
		${PROJECT_NAME}
		${ZLIB_LIBRARIES}
		${IRRLICHT_LIBRARY}
		${OPENGL_LIBRARIES}
		${JPEG_LIBRARIES}
		${BZIP2_LIBRARIES}
		${PNG_LIBRARIES}
		${X11_LIBRARIES}
		${GETTEXT_LIBRARY}
		${SOUND_LIBRARIES}
		${SQLITE3_LIBRARY}
		${LUA_LIBRARY}
		${GMP_LIBRARY}
		${JSON_LIBRARY}
		${OPENGLES2_LIBRARIES}
		${PLATFORM_LIBS}
		${CLIENT_PLATFORM_LIBS}
	)
	if(APPLE)
		target_link_libraries(
			${client_LIBS}
			${ICONV_LIBRARY}
		)
	else()
		target_link_libraries(
			${client_LIBS}
		)
	endif()
	if(USE_CURL)
		target_link_libraries(
			${PROJECT_NAME}
			${CURL_LIBRARY}
		)
	endif()
	if(USE_FREETYPE)
		if(FREETYPE_PKGCONFIG_FOUND)
			set_target_properties(${PROJECT_NAME}
				PROPERTIES
				COMPILE_FLAGS "${FREETYPE_CFLAGS_STR}"
			)
		endif()
		target_link_libraries(
			${PROJECT_NAME}
			${FREETYPE_LIBRARY}
		)
	endif()
	if (USE_CURSES)
		target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})
	endif()
	if (USE_POSTGRESQL)
		target_link_libraries(${PROJECT_NAME} ${POSTGRESQL_LIBRARY})
	endif()
	if (USE_LEVELDB)
		target_link_libraries(${PROJECT_NAME} ${LEVELDB_LIBRARY})
	endif()
	if (USE_REDIS)
		target_link_libraries(${PROJECT_NAME} ${REDIS_LIBRARY})
	endif()
	if (USE_SPATIAL)
		target_link_libraries(${PROJECT_NAME} ${SPATIAL_LIBRARY})
	endif()
endif(BUILD_CLIENT)


if(BUILD_SERVER)
	add_executable(${PROJECT_NAME}server ${server_SRCS} ${extra_windows_SRCS})
	add_dependencies(${PROJECT_NAME}server GenerateVersion)
	target_link_libraries(
		${PROJECT_NAME}server
		${ZLIB_LIBRARIES}
		${SQLITE3_LIBRARY}
		${JSON_LIBRARY}
		${GETTEXT_LIBRARY}
		${LUA_LIBRARY}
		${GMP_LIBRARY}
		${PLATFORM_LIBS}
	)
	set_target_properties(${PROJECT_NAME}server PROPERTIES
			COMPILE_DEFINITIONS "SERVER")
	if (USE_CURSES)
		target_link_libraries(${PROJECT_NAME}server ${CURSES_LIBRARIES})
	endif()
	if (USE_POSTGRESQL)
		target_link_libraries(${PROJECT_NAME}server ${POSTGRESQL_LIBRARY})
	endif()
	if (USE_LEVELDB)
		target_link_libraries(${PROJECT_NAME}server ${LEVELDB_LIBRARY})
	endif()
	if (USE_REDIS)
		target_link_libraries(${PROJECT_NAME}server ${REDIS_LIBRARY})
	endif()
	if (USE_SPATIAL)
		target_link_libraries(${PROJECT_NAME}server ${SPATIAL_LIBRARY})
	endif()
	if(USE_CURL)
		target_link_libraries(
			${PROJECT_NAME}server
			${CURL_LIBRARY}
		)
	endif()
endif(BUILD_SERVER)

