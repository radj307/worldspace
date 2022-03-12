/**
 * This file was automatically generated by VersionTag.cmake
 */
#pragma once
// @brief	Current worldspace2 major version number.
#define worldspace2_VERSION_MAJOR				0
// @brief	Current worldspace2 minor version number.
#define worldspace2_VERSION_MINOR				0
// @brief	Current worldspace2 patch version number.
#define worldspace2_VERSION_PATCH				1
// @brief	Current worldspace2 revision/build version number.
#define worldspace2_VERSION_EXTRA1			

#define worldspace2_VERSION_EXTRA2			
#define worldspace2_VERSION_EXTRA3			
#define worldspace2_VERSION_EXTRA4			
#define worldspace2_VERSION_EXTRA5			
#define worldspace2_VERSION_EXTRA6			
#define worldspace2_VERSION_EXTRA7			
#define worldspace2_VERSION_EXTRA8			
#define worldspace2_VERSION_EXTRA9			

// Define separators & stringize functions if they aren't overridden.
#if !defined(VERSION_H__SEPARATOR)
#	define VERSION_H__SEPARATOR				.
#endif
#if !defined(VERSION_H__SEPARATOR_EXTRA)
#	define VERSION_H__SEPERATOR_EXTRA		.
#endif

#if !defined(VERSION_H__STRINGIZE_) && !defined(VERSION_H__STRINGIZE)
#	define VERSION_H__STRINGIZE_(v)			#v
#	define VERSION_H__STRINGIZE(v)			VERSION_H__STRINGIZE_(v)
#endif

// @brief	Current worldspace2 version number as a string.
#define worldspace2_VERSION					VERSION_H__STRINGIZE(worldspace2_VERSION_MAJOR) VERSION_H__STRINGIZE(VERSION_H__SEPARATOR) VERSION_H__STRINGIZE(worldspace2_VERSION_MINOR) VERSION_H__STRINGIZE(VERSION_H__SEPARATOR) VERSION_H__STRINGIZE(worldspace2_VERSION_PATCH)
#define worldspace2_LITERAL_VERSION			"0.0.1"

// @brief	Current worldspace2 full version number as a string, including all extra components.
#define worldspace2_FULL_VERSION				worldspace2_VERSION VERSION_H__STRINGIZE(VERSION_H__SEPERATOR_EXTRA) VERSION_H_STRINGIZE(worldspace2_VERSION_EXTRA1) VERSION_H__STRINGIZE(VERSION_H__SEPERATOR_EXTRA) VERSION_H_STRINGIZE(worldspace2_VERSION_EXTRA2) VERSION_H__STRINGIZE(VERSION_H__SEPERATOR_EXTRA) VERSION_H_STRINGIZE(worldspace2_VERSION_EXTRA3) VERSION_H__STRINGIZE(VERSION_H__SEPERATOR_EXTRA) VERSION_H_STRINGIZE(worldspace2_VERSION_EXTRA4) VERSION_H__STRINGIZE(VERSION_H__SEPERATOR_EXTRA) VERSION_H_STRINGIZE(worldspace2_VERSION_EXTRA5) VERSION_H__STRINGIZE(VERSION_H__SEPERATOR_EXTRA) VERSION_H_STRINGIZE(worldspace2_VERSION_EXTRA6) VERSION_H__STRINGIZE(VERSION_H__SEPERATOR_EXTRA) VERSION_H_STRINGIZE(worldspace2_VERSION_EXTRA7) VERSION_H__STRINGIZE(VERSION_H__SEPERATOR_EXTRA) VERSION_H_STRINGIZE(worldspace2_VERSION_EXTRA8) VERSION_H__STRINGIZE(VERSION_H__SEPERATOR_EXTRA) VERSION_H_STRINGIZE(worldspace2_VERSION_EXTRA9)