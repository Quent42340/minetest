option(ENABLE_CURL "Enable cURL support for fetching media" TRUE)
set(USE_CURL FALSE)

if(ENABLE_CURL)
	find_package(CURL)
	if (CURL_FOUND)
		message(STATUS "cURL support enabled.")
		set(USE_CURL TRUE)
	endif()
else()
	mark_as_advanced(CLEAR CURL_LIBRARY CURL_INCLUDE_DIR)
endif()

if(NOT USE_CURL)
	if(BUILD_CLIENT)
		message(WARNING "cURL is required to load the server list")
	endif()
	if(BUILD_SERVER)
		message(WARNING "cURL is required to announce to the server list")
	endif()
endif()

option(ENABLE_GETTEXT "Use GetText for internationalization" TRUE)
set(USE_GETTEXT FALSE)

if(ENABLE_GETTEXT)
	find_package(GettextLib)
	if(GETTEXT_FOUND)
		if(WIN32)
			message(STATUS "GetText library: ${GETTEXT_LIBRARY}")
			message(STATUS "GetText DLL: ${GETTEXT_DLL}")
			message(STATUS "GetText iconv DLL: ${GETTEXT_ICONV_DLL}")
		endif()
		set(USE_GETTEXT TRUE)
		message(STATUS "GetText enabled; locales found: ${GETTEXT_AVAILABLE_LOCALES}")
	endif(GETTEXT_FOUND)
else()
	mark_as_advanced(GETTEXT_ICONV_DLL GETTEXT_INCLUDE_DIR GETTEXT_LIBRARY GETTEXT_MSGFMT)
	message(STATUS "GetText disabled.")
endif()

option(ENABLE_SOUND "Enable sound" TRUE)
set(USE_SOUND FALSE)

if(BUILD_CLIENT AND ENABLE_SOUND)
	# Sound libraries
	find_package(OpenAL)
	find_package(Vorbis)
	if(NOT OPENAL_FOUND)
		message(STATUS "Sound enabled, but OpenAL not found!")
		mark_as_advanced(CLEAR OPENAL_LIBRARY OPENAL_INCLUDE_DIR)
	endif()
	if(NOT VORBIS_FOUND)
		message(STATUS "Sound enabled, but Vorbis libraries not found!")
		mark_as_advanced(CLEAR OGG_INCLUDE_DIR VORBIS_INCLUDE_DIR OGG_LIBRARY VORBIS_LIBRARY VORBISFILE_LIBRARY)
	endif()
	if(OPENAL_FOUND AND VORBIS_FOUND)
		set(USE_SOUND TRUE)
		message(STATUS "Sound enabled.")
	else()
		message(FATAL_ERROR "Sound enabled, but cannot be used.\n"
			"To continue, either fill in the required paths or disable sound. (-DENABLE_SOUND=0)")
	endif()
endif()

option(ENABLE_GLES "Enable OpenGL ES support" FALSE)
mark_as_advanced(ENABLE_GLES)
if(ENABLE_GLES)
	find_package(OpenGLES2)
endif()

option(ENABLE_FREETYPE "Enable FreeType2 (TrueType fonts and basic unicode support)" TRUE)
set(USE_FREETYPE FALSE)

if(ENABLE_FREETYPE)
##
## Note: FindFreetype.cmake seems to have been fixed in recent versions of
## CMake. If issues persist, re-enable this workaround specificially for the
## failing platforms.
##
#	if(UNIX)
#		include(FindPkgConfig)
#		if(PKG_CONFIG_FOUND)
#			pkg_check_modules(FREETYPE QUIET freetype2)
#			if(FREETYPE_FOUND)
#				SET(FREETYPE_PKGCONFIG_FOUND TRUE)
#				SET(FREETYPE_LIBRARY ${FREETYPE_LIBRARIES})
#				# Because CMake is idiotic
#				string(REPLACE ";" " " FREETYPE_CFLAGS_STR ${FREETYPE_CFLAGS})
#				string(REPLACE ";" " " FREETYPE_LDFLAGS_STR ${FREETYPE_LDFLAGS})
#			endif(FREETYPE_FOUND)
#		endif(PKG_CONFIG_FOUND)
#	endif(UNIX)
#	if(NOT FREETYPE_FOUND)
#		find_package(Freetype)
#	endif()
	find_package(Freetype)
	if(FREETYPE_FOUND)
		message(STATUS "Freetype enabled.")
		set(USE_FREETYPE TRUE)
	endif()
endif(ENABLE_FREETYPE)

option(ENABLE_CURSES "Enable ncurses console" TRUE)
set(USE_CURSES FALSE)

if(ENABLE_CURSES)
	find_package(Ncursesw)
	if(CURSES_FOUND)
		set(USE_CURSES TRUE)
		message(STATUS "ncurses console enabled.")
		include_directories(${CURSES_INCLUDE_DIRS})
	else()
		message(STATUS "ncurses not found!")
	endif()
endif(ENABLE_CURSES)

option(ENABLE_POSTGRESQL "Enable PostgreSQL backend" TRUE)
set(USE_POSTGRESQL FALSE)

if(ENABLE_POSTGRESQL)
	find_program(POSTGRESQL_CONFIG_EXECUTABLE pg_config DOC "pg_config")
	find_library(POSTGRESQL_LIBRARY pq)
	if(POSTGRESQL_CONFIG_EXECUTABLE)
		execute_process(COMMAND ${POSTGRESQL_CONFIG_EXECUTABLE} --includedir-server
			OUTPUT_VARIABLE POSTGRESQL_SERVER_INCLUDE_DIRS
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		execute_process(COMMAND ${POSTGRESQL_CONFIG_EXECUTABLE}
			OUTPUT_VARIABLE POSTGRESQL_CLIENT_INCLUDE_DIRS
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		# This variable is case sensitive for the cmake PostgreSQL module
		set(PostgreSQL_ADDITIONAL_SEARCH_PATHS ${POSTGRESQL_SERVER_INCLUDE_DIRS} ${POSTGRESQL_CLIENT_INCLUDE_DIRS})
	endif()

	find_package("PostgreSQL")

	if(POSTGRESQL_FOUND)
		set(USE_POSTGRESQL TRUE)
		message(STATUS "PostgreSQL backend enabled")
		# This variable is case sensitive, don't try to change it to POSTGRESQL_INCLUDE_DIR
		message(STATUS "PostgreSQL includes: ${PostgreSQL_INCLUDE_DIR}")
		include_directories(${PostgreSQL_INCLUDE_DIR})
	else()
		message(STATUS "PostgreSQL not found!")
	endif()
endif(ENABLE_POSTGRESQL)

option(ENABLE_LEVELDB "Enable LevelDB backend" TRUE)
set(USE_LEVELDB FALSE)

if(ENABLE_LEVELDB)
	find_library(LEVELDB_LIBRARY leveldb)
	find_path(LEVELDB_INCLUDE_DIR db.h PATH_SUFFIXES leveldb)
	if(LEVELDB_LIBRARY AND LEVELDB_INCLUDE_DIR)
		set(USE_LEVELDB TRUE)
		message(STATUS "LevelDB backend enabled.")
		include_directories(${LEVELDB_INCLUDE_DIR})
	else()
		message(STATUS "LevelDB not found!")
	endif()
endif(ENABLE_LEVELDB)

OPTION(ENABLE_REDIS "Enable Redis backend" TRUE)
set(USE_REDIS FALSE)

if(ENABLE_REDIS)
	find_library(REDIS_LIBRARY hiredis)
	find_path(REDIS_INCLUDE_DIR hiredis.h PATH_SUFFIXES hiredis)
	if(REDIS_LIBRARY AND REDIS_INCLUDE_DIR)
		set(USE_REDIS TRUE)
		message(STATUS "Redis backend enabled.")
		include_directories(${REDIS_INCLUDE_DIR})
	else(REDIS_LIBRARY AND REDIS_INCLUDE_DIR)
		message(STATUS "Redis not found!")
	endif(REDIS_LIBRARY AND REDIS_INCLUDE_DIR)
endif(ENABLE_REDIS)

find_package(SQLite3 REQUIRED)

OPTION(ENABLE_SPATIAL "Enable SpatialIndex AreaStore backend" TRUE)
set(USE_SPATIAL FALSE)

if(ENABLE_SPATIAL)
	find_library(SPATIAL_LIBRARY spatialindex)
	find_path(SPATIAL_INCLUDE_DIR spatialindex/SpatialIndex.h)
	if(SPATIAL_LIBRARY AND SPATIAL_INCLUDE_DIR)
		set(USE_SPATIAL TRUE)
		message(STATUS "SpatialIndex AreaStore backend enabled.")
		include_directories(${SPATIAL_INCLUDE_DIR})
	else(SPATIAL_LIBRARY AND SPATIAL_INCLUDE_DIR)
		message(STATUS "SpatialIndex not found!")
	endif(SPATIAL_LIBRARY AND SPATIAL_INCLUDE_DIR)
endif(ENABLE_SPATIAL)

if(WIN32)
	# Windows
	if(MSVC) # MSVC Specifics
		set(PLATFORM_LIBS dbghelp.lib ${PLATFORM_LIBS})
		# Surpress some useless warnings
		add_definitions ( /D "_CRT_SECURE_NO_DEPRECATE" /W1 )
		# Get M_PI to work
		add_definitions(/D "_USE_MATH_DEFINES")
		# Dont define min/max macros in minwindef.h
		add_definitions(/D "NOMINMAX")
	else() # Probably MinGW = GCC
		set(PLATFORM_LIBS "")
	endif()
	set(PLATFORM_LIBS ws2_32.lib version.lib shlwapi.lib ${PLATFORM_LIBS})

	# Zlib stuff
	set(ZLIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../../zlib/zlib-1.2.5"
			CACHE PATH "Zlib include directory")
	set(ZLIB_LIBRARIES "${PROJECT_SOURCE_DIR}/../../zlib125dll/dll32/zlibwapi.lib"
			CACHE FILEPATH "Path to zlib library (usually zlibwapi.lib)")
	set(ZLIB_DLL "${PROJECT_SOURCE_DIR}/../../zlib125dll/dll32/zlibwapi.dll"
			CACHE FILEPATH "Path to zlib DLL (for installation)")
	set(ZLIBWAPI_DLL "" CACHE FILEPATH "Path to zlibwapi DLL")
	set(IRRLICHT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../../irrlicht-1.7.2"
			CACHE PATH "irrlicht dir")
	if(USE_FREETYPE)
		set(FREETYPE_INCLUDE_DIR_ft2build "${PROJECT_SOURCE_DIR}/../../freetype2/include/"
				CACHE PATH "freetype include dir")
		set(FREETYPE_INCLUDE_DIR_freetype2 "${PROJECT_SOURCE_DIR}/../../freetype2/include/freetype"
				CACHE PATH "freetype include dir")
		set(FREETYPE_LIBRARY "${PROJECT_SOURCE_DIR}/../../freetype2/objs/win32/vc2005/freetype247.lib"
				CACHE FILEPATH "Path to freetype247.lib")
	endif()
	if(ENABLE_SOUND)
		set(OPENAL_DLL "" CACHE FILEPATH "Path to OpenAL32.dll for installation (optional)")
		set(OGG_DLL "" CACHE FILEPATH "Path to libogg.dll for installation (optional)")
		set(VORBIS_DLL "" CACHE FILEPATH "Path to libvorbis.dll for installation (optional)")
		set(VORBISFILE_DLL "" CACHE FILEPATH "Path to libvorbisfile.dll for installation (optional)")
	endif()
	if(USE_LUAJIT)
		set(LUA_DLL "" CACHE FILEPATH "Path to lua51.dll for installation (optional)")
	endif()
else()
	# Unix probably
	if(BUILD_CLIENT)
		if(NOT HAIKU)
			find_package(X11 REQUIRED)
		endif(NOT HAIKU)

		set(OPENGL_GL_PREFERENCE "LEGACY" CACHE STRING
			"See CMake Policy CMP0072 for reference. GLVND is broken on some nvidia setups")
		set(OpenGL_GL_PREFERENCE ${OPENGL_GL_PREFERENCE})

		find_package(OpenGL REQUIRED)
		find_package(JPEG REQUIRED)
		find_package(BZip2 REQUIRED)
		find_package(PNG REQUIRED)
		if(APPLE)
			find_library(CARBON_LIB Carbon)
			find_library(COCOA_LIB Cocoa)
			find_library(IOKIT_LIB IOKit)
			mark_as_advanced(
				CARBON_LIB
				COCOA_LIB
				IOKIT_LIB
			)
			SET(CLIENT_PLATFORM_LIBS ${CLIENT_PLATFORM_LIBS} ${CARBON_LIB} ${COCOA_LIB} ${IOKIT_LIB})
		endif(APPLE)
	endif(BUILD_CLIENT)
	find_package(ZLIB REQUIRED)
	set(PLATFORM_LIBS -lpthread ${CMAKE_DL_LIBS})
	if(APPLE)
		set(PLATFORM_LIBS "-framework CoreFoundation" ${PLATFORM_LIBS})
	else()
		check_library_exists(rt clock_gettime "" HAVE_LIBRT)
		if (HAVE_LIBRT)
			set(PLATFORM_LIBS -lrt ${PLATFORM_LIBS})
		endif(HAVE_LIBRT)
	endif(APPLE)

	if(NOT HAIKU AND NOT APPLE)
	# This way Xxf86vm is found on OpenBSD too
		find_library(XXF86VM_LIBRARY Xxf86vm)
		mark_as_advanced(XXF86VM_LIBRARY)
		set(CLIENT_PLATFORM_LIBS ${CLIENT_PLATFORM_LIBS} ${XXF86VM_LIBRARY})
	endif(NOT HAIKU AND NOT APPLE)

	# Prefer local iconv if installed
	find_library(ICONV_LIBRARY iconv)
	mark_as_advanced(ICONV_LIBRARY)
	if (ICONV_LIBRARY)
		set(PLATFORM_LIBS ${PLATFORM_LIBS} ${ICONV_LIBRARY})
	endif()
endif()

if(USE_FREETYPE)
	include_directories(${FREETYPE_INCLUDE_DIRS})
endif()

if(USE_CURL)
	include_directories(${CURL_INCLUDE_DIR})
endif()

