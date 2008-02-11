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

// Windows specific switches
#ifdef WIN32
	// appears to be required by LibXUL/Mozilla code to avoid crashes in debug versions of their code (undef'd at end of this file)
	#ifdef _DEBUG
		#define DEBUG 1
	#endif
#endif	// WIN32

#include "Browser.h"
#include "MozillaBrowserWindow.h"

#ifdef WIN32
	#pragma warning( disable : 4265 )	// "class has virtual functions, but destructor is not virtual"
	#pragma warning( disable : 4291 )	// (no matching operator delete found; memory will not be freed if initialization throws an exception)
#endif	// WIN32

#include "nsBuildID.h"
#include "nsICacheService.h"
#include "nsIPref.h"
#include "nsNetCID.h"
#include "nsProfileDirServiceProvider.h"
#include "nsXULAppAPI.h"

using namespace NMozLib;

Browser* Browser::instance = 0;

Browser::Browser(const std::string& runtimeDir, const std::string& profileDir)
{
	nsCOMPtr<nsILocalFile> appDir;
	nsresult result = NS_NewNativeLocalFile(nsCString(runtimeDir.c_str()), PR_FALSE, getter_AddRefs(appDir));
	if(NS_FAILED(result))
		throw std::exception("NMozLib::Browser, invalid runtime directory specified.");

	result = XRE_InitEmbedding(appDir, appDir, nsnull, nsnull, 0);
	if(NS_FAILED(result))
		throw std::exception("NMozLib::Browser, unable to initialize XULRunner using the specified runtime directory.");

	nsCOMPtr<nsILocalFile> appDataDir;
	NS_NewNativeLocalFile(nsCString(runtimeDir.c_str()), PR_TRUE, getter_AddRefs(appDataDir));
	result = appDataDir->Append(NS_ConvertUTF8toUCS2(profileDir.c_str()));
	if(NS_FAILED(result))
		throw std::exception("NMozLib::Browser, invalid profile directory specified.");

	nsCOMPtr<nsILocalFile> localAppDataDir(do_QueryInterface(appDataDir));

	nsCOMPtr<nsProfileDirServiceProvider> locProvider;
	result = NS_NewProfileDirServiceProvider(PR_TRUE, getter_AddRefs(locProvider));
	if(NS_FAILED(result))
		throw std::exception("NMozLib::Browser, unable to instantiate ProfileDirServiceProvider.");

	result = locProvider->Register();
	if(NS_FAILED(result))
		throw std::exception("NMozLib::Browser, unable to register ProfileDirServiceProvider.");

	result = locProvider->SetProfileDir(localAppDataDir);
	if(NS_FAILED(result))
		throw std::exception("NMozLib::Browser, unable to set profile directory.");

	// TODO: temporary fix - need to do this in the absence of the relevant dialogs
	setBooleanPref("security.warn_entering_secure", false);
	setBooleanPref("security.warn_entering_weak", false);
	setBooleanPref("security.warn_leaving_secure", false);
	setBooleanPref("security.warn_submit_insecure", false);

	instance = this;
}

Browser::~Browser()
{
	for(WindowIter i = windows.begin(); i != windows.end();)
	{
		MozillaBrowserWindow* browserWin = static_cast<MozillaBrowserWindow*>(*i);
		i = windows.erase(i);
		browserWin->destroy();
	}

	XRE_TermEmbedding();

	instance = 0;
}

Browser& Browser::Get()
{
	if(!instance)
		throw std::exception("In NMozLib::Browser::Get, singleton hasn't been instantiated yet!");

	return *instance;
}

Browser* Browser::GetPointer()
{
	return instance;
}

BrowserWindow* Browser::createBrowserWindow(void* nativeWindowHandle, int width, int height)
{
	MozillaBrowserWindow* newBrowser = new MozillaBrowserWindow(nativeWindowHandle, width, height);
	
	windows.push_back(newBrowser);

	return newBrowser;
}

void Browser::destroyBrowserWindow(BrowserWindow* browserWindow)
{
	for(WindowIter i = windows.begin(); i != windows.end(); i++)
	{
		if(*i == browserWindow)
		{
			windows.erase(i);
			break;
		}
	}

	//delete static_cast<MozillaBrowserWindow*>(browserWindow);
	static_cast<MozillaBrowserWindow*>(browserWindow)->destroy();
}

bool Browser::setBooleanPref(const std::string& prefName, bool value)
{
	nsresult result;
	nsCOMPtr<nsIPref> pref = do_CreateInstance(NS_PREF_CONTRACTID, &result);
	if(NS_FAILED(result))
		return false;

	result = pref->SetBoolPref(prefName.c_str(), (PRBool)value);

	return NS_SUCCEEDED(result);
}

bool Browser::setIntegerPref(const std::string& prefName, int value)
{
	nsresult result;
	nsCOMPtr<nsIPref> pref = do_CreateInstance(NS_PREF_CONTRACTID, &result);
	if(NS_FAILED(result))
		return false;

	result = pref->SetIntPref(prefName.c_str(), (PRInt32)value);

	return NS_SUCCEEDED(result);
}

bool Browser::setStringPref(const std::string& prefName, const std::string& value)
{
	nsresult result;
	nsCOMPtr<nsIPref> pref = do_CreateInstance(NS_PREF_CONTRACTID, &result);
	if(NS_FAILED(result))
		return false;

	result = pref->SetCharPref(prefName.c_str(), value.c_str());

	return NS_SUCCEEDED(result);
}

void Browser::clearCache()
{
	nsresult result;
	nsCOMPtr<nsICacheService> cacheService = do_GetService(NS_CACHESERVICE_CONTRACTID, &result);
	if(NS_FAILED(result))
		return;

	cacheService->EvictEntries(nsICache::STORE_ANYWHERE);
}

std::string Browser::getVersion()
{
	return std::string(GRE_BUILD_ID);
}

// Windows specific switches
#ifdef WIN32
	// #define required by this file for LibXUL/Mozilla code to avoid crashes in their debug code
	#ifdef _DEBUG
		#undef DEBUG
	#endif

#endif	// WIN32