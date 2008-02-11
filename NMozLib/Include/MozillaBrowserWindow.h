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

#ifndef __MozillaBrowserWindow_H__
#define __MozillaBrowserWindow_H__

// Mozilla code has non-virtual destructors
#ifdef WIN32
#pragma warning( disable : 4265 ) // "class has virtual functions, but destructor is not virtual"
#endif

#include "nsIBaseWindow.h"
#include "nsIDOMEventListener.h"
#include "nsIDOMEventTarget.h"
#include "nsIInterfaceRequestor.h"
#include "nsIWebBrowserChrome.h"
#include "nsIWebNavigation.h"
#include "nsIWebProgressListener.h"
#include "nsIURIContentListener.h"
#include "nsWeakReference.h"
#include "nsIWebBrowser.h"
#include "nsIToolkit.h"
#include "nsIScriptGlobalObject.h"
#include "nsIScriptGlobalObjectOwner.h"
#include "nsIScriptContext.h"
#include <string>
#include <vector>
#include <algorithm>
#include "BrowserWindow.h"

namespace NMozLib {

class MozillaBrowserWindow : public BrowserWindow, public nsIInterfaceRequestor, public nsIWebBrowserChrome, public nsIWebProgressListener,
	public nsIURIContentListener, public nsSupportsWeakReference, public nsIDOMEventListener, public nsIToolkitObserver
{
public:

	MozillaBrowserWindow(void* nativeWindowHandle, int width, int height);
	virtual ~MozillaBrowserWindow();

	void destroy();

	NS_DECL_ISUPPORTS
	NS_DECL_NSIINTERFACEREQUESTOR
	NS_DECL_NSIWEBBROWSERCHROME
	NS_DECL_NSIWEBPROGRESSLISTENER
	NS_DECL_NSIURICONTENTLISTENER
	NS_DECL_NSIDOMEVENTLISTENER
	NS_DECL_NSITOOLKITOBSERVER

	void navigateTo(const std::string& url);
	void navigateStop();
	void navigateRefresh();
	void navigateForward();
	void navigateBack();
	
	bool canNavigateForward() const;
	bool canNavigateBack() const;

	std::string evaluateJS(const std::string& script);

	void focus();
	void defocus();

	void injectMouseMove(short x, short y);
	void injectMouseDown(short x, short y);
	void injectMouseUp(short x, short y);
	void injectKeyPress(short keyCode);
	void injectScroll(short numLines);

	void addListener(BrowserWindowListener* listener);
	void removeListener(BrowserWindowListener* listener);

	bool render();
	const RenderSurface& getRenderSurface() const;

	CaretInfo getCaretInfo() const;

	std::string getCurrentURL() const;

	void resize(short width, short height);

	void setBackgroundColor(unsigned char red, unsigned char green, unsigned char blue);

protected:
	PRBool sendMozillaMouseEvent( PRInt16 eventIn, PRInt16 xPosIn, PRInt16 yPosIn );
	PRBool sendMozillaKeyboardEvent( PRUint32 keyIn, PRUint32 ns_vk_code );

	nsCOMPtr<nsIWebBrowser> webBrowser;
	nsCOMPtr<nsIBaseWindow> baseWindow;
	nsCOMPtr<nsIWebNavigation> webNav;

	RenderSurface surface;

	std::vector<BrowserWindowListener*> listeners;
	typedef std::vector<BrowserWindowListener*>::iterator ListenerIter;

	bool okayToRender;
	std::string currentURL;
	int bgColor;
};

}

#endif