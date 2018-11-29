file(GLOB_RECURSE player_core_SRCS ABSOLUTE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")

set(player_SRCS
	${player_core_SRCS}
	PARENT_SCOPE
)

