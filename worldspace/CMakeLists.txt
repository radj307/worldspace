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
MAKE_VERSION_HEADER("${CMAKE_CURRENT_SOURCE_DIR}/version.h" "${WORLDSPACE_VERSION}")

# Create an executable
add_executable(worldspace "${SRCS}")

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
INSTALL_EXECUTABLE(worldspace)
