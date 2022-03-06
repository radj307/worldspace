# worldspace/
cmake_minimum_required(VERSION 3.20)

file(GLOB HEADERS
	RELATIVE "${CURRENT_CMAKE_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.h*"
)
file(GLOB SRCS
	RELATIVE "${CURRENT_CMAKE_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.c*"
)

include(VersionTag)

file(REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/version.h")
MAKE_VERSION_HEADER("${CMAKE_CURRENT_SOURCE_DIR}/version.h" "WORLDSPACE" "${WORLDSPACE_VERSION}")

if (WIN32)
	include(ResourceMaker)

	set(WORLDSPACE_RESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/worldspace.rc")
	file(REMOVE "${WORLDSPACE_RESOURCE_FILE}")
	CREATE_VERSION_RESOURCE(
			"${WORLDSPACE_RESOURCE_FILE}"
			"${WORLDSPACE_VERSION_MAJOR}"
			"${WORLDSPACE_VERSION_MINOR}"
			"${WORLDSPACE_VERSION_PATCH}"
			"radj307"
			"Console-based Live-action Roguelike RPG"
			"worldspace"
			"GPLv3"
			"worldspace.exe"
			"worldspace"
	)
	APPEND_ICON_RESOURCE("${WORLDSPACE_RESOURCE_FILE}" "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/MAINICON.ico")
endif()

# Create an executable
add_executable(worldspace "${SRCS}" "${WORLDSPACE_RESOURCE_FILE}")

# Set C++ properties
set_property(TARGET worldspace PROPERTY CXX_STANDARD 20)
set_property(TARGET worldspace PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET worldspace PROPERTY POSITION_INDEPENDENT_CODE ON)
if (MSVC)
	target_compile_options(worldspace PUBLIC "/Zc:__cplusplus")
endif()

# Include source files
target_sources(worldspace PUBLIC "${HEADERS}")

# Link dependencies
target_link_libraries(worldspace PUBLIC shared TermAPI strlib optlib filelib)

# Create an install target
include(PackageInstaller)
INSTALL_EXECUTABLE(worldspace "${CMAKE_INSTALL_PREFIX}/bin")
