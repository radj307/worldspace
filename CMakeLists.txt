# worldspace/
cmake_minimum_required(VERSION 3.20)

# Add modules path:
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/307lib/307modules")

# Get the current version number from the latest git tag:
include(VersionTag)
set(ENV{WORLDSPACE_VERSION} "0.0.0")
GET_VERSION_TAG("${CMAKE_CURRENT_LIST_DIR}" "WORLDSPACE")

project("worldspace" VERSION "${WORLDSPACE_VERSION}" LANGUAGES CXX)

# add subdirectories

add_subdirectory("307lib")

option(USE_LEGACY_WORLDSPACE "When enabled, the original worldspace project is built. It may not be fully functional." OFF)

if (USE_LEGACY_WORLDSPACE)
	add_subdirectory("worldspace")
else()
	add_subdirectory("worldspace2")
endif()
