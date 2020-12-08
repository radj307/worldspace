/****************************************************************************************
 *									INI_Include.h										*
 *		Include this header file & define default settings to use in a project.			*
 *									 by radj307											*
 *																						*
 *					  Include all expected INI settings below.							*
 *		   Any setting in the file that does not exist here will be ignored.			*
 *	 Any settings not found in the file that exist here will be added automatically.	*
 *		     You can override this by removing the line "#define INI_DEF_H"				*
 ****************************************************************************************/
#pragma once
// Include vectors
#include <vector>
// Include base ini settings
#include "INI_Setting.h"

// Define the following:
const std::string __INI_NAME{ "worldspace.ini" };
inline const std::vector<INI_Setting*> __DEF{ // Put your default settings here:
	new iINI_Setting("Key_Up", static_cast<int>('w'), "Key to move up"),
	new iINI_Setting("Key_Down", static_cast<int>('s'), "Key to move down"),
	new iINI_Setting("Key_Left", static_cast<int>('a'), "Key to move left"),
	new iINI_Setting("Key_Right", static_cast<int>('d'), "Key to move right"),
	new iINI_Setting("Key_Pause", static_cast<int>('p'), "Key to pause/unpause the game"),
	new iINI_Setting("Key_Quit", static_cast<int>('q'), "Key to quit the game"),
};
// Set compiler def to turn off override defaults
#define INI_DEF_H

// Include the main INI file once defaults have been set
#include "INI.h"

// Create INI config, load from INI name. (DEFAULT NAME: "cfg")
inline INI cfg(__INI_NAME);