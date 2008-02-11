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

#ifndef __BrowserWindow_H__
#define __BrowserWindow_H__

#include <string>

namespace NMozLib {

class BrowserWindowListener;
class RenderSurface;
struct CaretInfo;

class BrowserWindow
{
public:
	virtual ~BrowserWindow() {}

	virtual void navigateTo(const std::string& url) = 0;
	virtual void navigateStop() = 0;
	virtual void navigateRefresh() = 0;
	virtual void navigateForward() = 0;
	virtual void navigateBack() = 0;
	
	virtual bool canNavigateForward() const = 0;
	virtual bool canNavigateBack() const = 0;

	virtual std::string evaluateJS(const std::string& script) = 0;

	virtual void focus() = 0;
	virtual void defocus() = 0;

	virtual void injectMouseMove(short x, short y) = 0;
	virtual void injectMouseDown(short x, short y) = 0;
	virtual void injectMouseUp(short x, short y) = 0;
	virtual void injectKeyPress(short keyCode) = 0;
	virtual void injectScroll(short numLines) = 0;

	virtual void addListener(BrowserWindowListener* listener) = 0;
	virtual void removeListener(BrowserWindowListener* listener) = 0;

	virtual bool render() = 0;
	virtual const RenderSurface& getRenderSurface() const = 0;

	virtual CaretInfo getCaretInfo() const = 0;

	virtual std::string getCurrentURL() const = 0;

	virtual void resize(short width, short height) = 0;

	virtual void setBackgroundColor(unsigned char red, unsigned char green, unsigned char blue) = 0;
};

class BrowserWindowListener
{
public:
	virtual void onPageChanged(BrowserWindow* caller, int x, int y, int width, int height) = 0;
	virtual void onNavigateBegin(BrowserWindow* caller, const std::string& url, bool &shouldContinue) = 0;
	virtual void onNavigateComplete(BrowserWindow* caller, const std::string& url, int responseCode) = 0;
	virtual void onUpdateProgress(BrowserWindow* caller, short percentComplete) = 0;
	virtual void onStatusTextChange(BrowserWindow* caller, const std::string& statusText) = 0;
	virtual void onLocationChange(BrowserWindow* caller, const std::string& url) = 0;
	virtual void onClickLinkHref(BrowserWindow* caller, const std::string& linkHref) = 0;
};

class RenderSurface
{
public:
	RenderSurface();
	RenderSurface(short width, short height, short depth = 4);

	~RenderSurface();

	void resize(short width, short height, short depth, int rowPitch);

	unsigned char* buffer;
	short width;
	short height;
	short depth;
	int rowPitch;
};

struct CaretInfo
{
	bool visible;
	int x, y, height;

	CaretInfo();
	CaretInfo(bool visible, int x, int y, int height);

	bool operator==(const CaretInfo& rhs) const;
	bool operator!=(const CaretInfo& rhs) const;
};

}

#endif