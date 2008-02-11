/*
	This file is part of NMozLib, "Navi Mozilla Library", a wrapper for Mozilla Gecko

	Copyright (C) 2008 Adam J. Simmons
	http://code.google.com/p/nmozlib/

	Portions Copyright (C) 2006 Callum Prentice and Linden Lab Inc.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __Browser_H__
#define __Browser_H__

#include <string>
#include <vector>
#include "BrowserWindow.h"

namespace NMozLib {

class Browser
{
public:
	Browser(const std::string& runtimeDir, const std::string& profileDir);
	~Browser();

	static Browser& Get();
	static Browser* GetPointer();

	BrowserWindow* createBrowserWindow(void* nativeWindowHandle, int width, int height);
	void destroyBrowserWindow(BrowserWindow* browserWindow);

	bool setBooleanPref(const std::string& prefName, bool value);
	bool setIntegerPref(const std::string& prefName, int value);
	bool setStringPref(const std::string& prefName, const std::string& value);

	void clearCache();

	std::string getVersion();
	
protected:
	static Browser* instance;
	std::vector<BrowserWindow*> windows;
	typedef std::vector<BrowserWindow*>::iterator WindowIter;
};

}

#endif