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

// seems to be required by LibXUL/Mozilla code to avoid crashes in their debug code, but only on Windows.
// undef'd at end of this
#ifdef _DEBUG
	#ifdef WIN32
		#define DEBUG 1
	#endif
#endif

// needed for the code in MozillaBrowserWindow::NotifyInvalidated() which will
// one day be moved to platform agnostic code when I find out how...
#ifdef WIN32
#include "windows.h"
#endif

#include "nsCWebBrowser.h"
#include "nsGUIEvent.h"
#include "nsICaret.h"
#include "nsIContent.h"
#include "nsIDOMDocument.h"
#include "nsIDOMElement.h"
#include "nsIDOMWindow.h"
#include "nsIDOMEvent.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsIDocument.h"
#include "nsIFrame.h"
#include "nsIHttpChannel.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIScrollableView.h"
#include "nsISelection.h"
#include "nsISelectionController.h"
#include "nsIWebBrowserChrome.h"
#include "nsIWebBrowserChromeFocus.h"
#include "nsIWebBrowserFocus.h"
#include "nsIWebProgress.h"
#include "nsIWebProgressListener.h"
#include "nsPresContext.h"
#include "nsProfileDirServiceProvider.h"
#include "nsXPCOMGlue.h"
#include "nsXULAppAPI.h"
#include "MozillaBrowserWindow.h"

using namespace NMozLib;

MozillaBrowserWindow::MozillaBrowserWindow(void* nativeWindowHandle, int width, int height) : bgColor(NS_RGB(255, 255, 255)), okayToRender(false)
{
	// Owns itself? Hack!
	this->AddRef();

	nsresult result;

	webBrowser = do_CreateInstance(NS_WEBBROWSER_CONTRACTID);

    webBrowser->SetContainerWindow(NS_STATIC_CAST(nsIWebBrowserChrome*, this));

    nsCOMPtr<nsIDocShellTreeItem> dsti = do_QueryInterface(webBrowser);
    dsti->SetItemType(nsIDocShellTreeItem::typeContentWrapper);

    baseWindow = do_QueryInterface(webBrowser);

    result = baseWindow->InitWindow(nativeWindowHandle, nsnull, 0, 0, width, height);
	if(NS_FAILED(result))
		throw std::exception("NMozLib::MozillaBrowserWindow, unable to initialize nsBaseWindow.");

	result = baseWindow->Create();
	if(NS_FAILED(result))
		throw std::exception("NMozLib::MozillaBrowserWindow, unable to create nsBaseWindow.");

	nsWeakPtr listenerRef(dont_AddRef(NS_GetWeakReference(NS_STATIC_CAST(nsIWebProgressListener*, this))));
    webBrowser->AddWebBrowserListener(listenerRef, NS_GET_IID(nsIWebProgressListener));

	webBrowser->SetParentURIContentListener(NS_STATIC_CAST(nsIURIContentListener*, this));

	baseWindow->SetVisibility(PR_FALSE);
	
	webNav = do_QueryInterface(webBrowser);

	resize(width, height);

	webNav->LoadURI(NS_ConvertUTF8toUTF16("about:blank").get(), nsIWebNavigation::LOAD_FLAGS_NONE, nsnull, nsnull, nsnull);
}

MozillaBrowserWindow::~MozillaBrowserWindow()
{

	//mRefCnt = 0;
}

void MozillaBrowserWindow::destroy()
{
	webNav->Stop(nsIWebNavigation::STOP_ALL);

	webBrowser->SetParentURIContentListener(0);
	nsWeakPtr listenerRef(dont_AddRef(NS_GetWeakReference(NS_STATIC_CAST(nsIWebProgressListener*, this))));
    webBrowser->RemoveWebBrowserListener(listenerRef, NS_GET_IID(nsIWebProgressListener));

	// Bah, calling this after another instance has destroyed its own baseWindow causes some nastiness
	//baseWindow->Destroy();

	webBrowser->SetContainerWindow(0);

	this->Release();
	this->Release(); // TODO: something is adding an extra ref somewhere, this is a hack
}

NS_IMPL_ADDREF( MozillaBrowserWindow )
NS_IMPL_RELEASE( MozillaBrowserWindow )

NS_INTERFACE_MAP_BEGIN( MozillaBrowserWindow )
	NS_INTERFACE_MAP_ENTRY_AMBIGUOUS( nsISupports, nsIWebBrowserChrome )
	NS_INTERFACE_MAP_ENTRY( nsIInterfaceRequestor )
	NS_INTERFACE_MAP_ENTRY( nsIWebBrowserChrome )
	NS_INTERFACE_MAP_ENTRY( nsIWebProgressListener )
	NS_INTERFACE_MAP_ENTRY( nsIURIContentListener )
	NS_INTERFACE_MAP_ENTRY( nsISupportsWeakReference )
	NS_INTERFACE_MAP_ENTRY( nsIToolkitObserver )
NS_INTERFACE_MAP_END

// nsIInterfaceRequestor Definitions

NS_IMETHODIMP MozillaBrowserWindow::GetInterface(const nsIID & uuid, void * *result)
{
	if(uuid.Equals(NS_GET_IID(nsIDOMWindow)))
	{
		if(webBrowser)
			return webBrowser->GetContentDOMWindow((nsIDOMWindow**)result);

		return NS_ERROR_NOT_INITIALIZED;
	}

	return QueryInterface(uuid, result);
}

// Begin nsIWebBrowserChrome Definitions

NS_IMETHODIMP MozillaBrowserWindow::SetStatus(PRUint32 statusType, const PRUnichar *status)
{
	std::string statusText(NS_ConvertUTF16toUTF8(status).get());

	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
		(*i)->onStatusTextChange(this, statusText);

	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::GetWebBrowser(nsIWebBrowser * *aWebBrowser)
{
	NS_ENSURE_ARG_POINTER(aWebBrowser);
	*aWebBrowser = webBrowser;
	NS_IF_ADDREF(*aWebBrowser);

	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::SetWebBrowser(nsIWebBrowser * aWebBrowser)
{
	NS_ENSURE_ARG_POINTER(aWebBrowser);
	webBrowser = aWebBrowser;

	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::GetChromeFlags(PRUint32 *aChromeFlags)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP MozillaBrowserWindow::SetChromeFlags(PRUint32 aChromeFlags)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP MozillaBrowserWindow::DestroyBrowserWindow(void)
{
	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::SizeBrowserTo(PRInt32 aCX, PRInt32 aCY)
{
	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::ShowAsModal(void)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP MozillaBrowserWindow::IsWindowModal(PRBool *_retval)
{
	*_retval = PR_FALSE;

	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::ExitModalEventLoop(nsresult aStatus)
{
	return NS_OK;
}

// Begin nsIWebProgressListener Definitions

NS_IMETHODIMP MozillaBrowserWindow::OnStateChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aStateFlags, nsresult aStatus)
{
 	if((aStateFlags & STATE_START) && (aStateFlags & STATE_IS_DOCUMENT) && (aStatus == NS_OK))
	{
		// page load is starting so remove listener that catches "click" events
		nsCOMPtr<nsIDOMWindow> window;
		nsresult result = aWebProgress->GetDOMWindow(getter_AddRefs(window));
		if(NS_SUCCEEDED(result))
		{
			nsCOMPtr<nsIDOMEventTarget> target = do_QueryInterface(window, &result);
			if(NS_SUCCEEDED(result))
				target->RemoveEventListener(NS_ConvertUTF8toUTF16("click"), this, PR_TRUE);
		}

		okayToRender = false;

		// possible new page, attempt to toggle toolkit observer
		nsCOMPtr<nsIDocShell> docShell = do_GetInterface(webBrowser);

		nsCOMPtr<nsPresContext> presContext;
		docShell->GetPresContext(getter_AddRefs(presContext));
		if(!presContext)
			return NS_OK;

		nsIViewManager* viewManager = presContext->GetViewManager();
		if(!viewManager)
			return NS_OK;

		nsIView* rootView = 0;
		viewManager->GetRootView(rootView);
		if(!rootView)
			return NS_OK;

		nsCOMPtr<nsIWidget> widget = rootView->GetWidget();
		if(!widget)
			return NS_OK;

		nsCOMPtr<nsIToolkit> toolkit = widget->GetToolkit();
		if(!toolkit)
			return NS_OK;

		toolkit->RemoveObserver(this);
		toolkit->AddObserver(this);
	}
	else if((aStateFlags & STATE_STOP) && (aStateFlags & STATE_IS_WINDOW) && (aStatus == NS_OK))
	{
		// page load is complete so add listener that catches "click" events
		nsCOMPtr<nsIDOMWindow> window;
		nsresult result = aWebProgress->GetDOMWindow(getter_AddRefs(window));
		if(NS_SUCCEEDED(result))
		{
			nsCOMPtr<nsIDOMEventTarget> target = do_QueryInterface(window, &result);
			if(NS_SUCCEEDED(result))
				target->AddEventListener(NS_ConvertUTF8toUTF16("click"), this, PR_TRUE);
		}

		// pick up raw HTML response status code
		PRUint32 responseStatus = 0;
		if(aRequest)
		{
			nsCOMPtr<nsIHttpChannel> httpChannel = do_QueryInterface(aRequest, &result);
			if(NS_SUCCEEDED(result))
				httpChannel->GetResponseStatus(&responseStatus);
		}

		for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
			(*i)->onNavigateComplete(this, getCurrentURL(), (int)responseStatus);

		okayToRender = true;
	}

	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::OnProgressChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt32 aCurSelfProgress, PRInt32 aMaxSelfProgress, PRInt32 aCurTotalProgress, PRInt32 aMaxTotalProgress)
{
	short percentComplete = (short)((aCurTotalProgress * 100.0f) / (float)aMaxTotalProgress);

	if(percentComplete < 0)
		percentComplete = 0;
	else if(percentComplete > 100)
		percentComplete = 100;

	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
		(*i)->onUpdateProgress(this, percentComplete);

	okayToRender = true;

	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::OnLocationChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsIURI *aLocation)
{
	nsCAutoString newURI;
	aLocation->GetSpec(newURI);
	currentURL = newURI.get();

	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
		(*i)->onLocationChange(this, currentURL);

	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::OnStatusChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsresult aStatus, const PRUnichar *aMessage)
{
	std::string statusText(NS_ConvertUTF16toUTF8(aMessage).get());

	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
		(*i)->onStatusTextChange(this, statusText);

	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::OnSecurityChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aState)
{
	return NS_OK;
}

// Begin nsIURIContentListener Definitions

NS_IMETHODIMP MozillaBrowserWindow::OnStartURIOpen(nsIURI *aURI, PRBool *_retval)
{
	nsCAutoString urlString;
	aURI->GetSpec(urlString);
	std::string url(urlString.get());

	bool abortURI = false;

	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
	{
		bool continueTest = true;
		(*i)->onNavigateBegin(this, url, continueTest);

		if(!continueTest)
			abortURI = true;
	}

	*_retval = abortURI;

	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::DoContent(const char *aContentType, PRBool aIsContentPreferred, nsIRequest *aRequest, nsIStreamListener **aContentHandler, PRBool *_retval)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP MozillaBrowserWindow::IsPreferred(const char *aContentType, char **aDesiredContentType, PRBool *_retval)
{
	// important (otherwise, links try to open in a new window and trigger the window watcher code)
	*_retval = PR_TRUE;
	return NS_OK;
}

NS_IMETHODIMP MozillaBrowserWindow::CanHandleContent(const char *aContentType, PRBool aIsContentPreferred, char **aDesiredContentType, PRBool *_retval)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP MozillaBrowserWindow::GetLoadCookie(nsISupports * *aLoadCookie)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP MozillaBrowserWindow::SetLoadCookie(nsISupports * aLoadCookie)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP MozillaBrowserWindow::GetParentContentListener(nsIURIContentListener * *aParentContentListener)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP MozillaBrowserWindow::SetParentContentListener(nsIURIContentListener * aParentContentListener)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

// Begin nsIDOMEventListener Definitions

NS_IMETHODIMP MozillaBrowserWindow::HandleEvent(nsIDOMEvent *event)
{
	nsCOMPtr<nsIDOMEventTarget> eventTarget;
	event->GetTarget(getter_AddRefs(eventTarget));

	nsCOMPtr<nsIDOMElement> linkElement(do_QueryInterface(eventTarget));

	if(linkElement)
	{
		nsString hrefAttribute;
		linkElement->GetAttribute(NS_ConvertUTF8toUTF16("href"), hrefAttribute);

		std::string linkHref(NS_ConvertUTF16toUTF8(hrefAttribute).get());

		for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
			(*i)->onClickLinkHref(this, linkHref);
	}

	return NS_OK;
} 

// Begin nsIToolkitObserver Definitions

NS_IMETHODIMP MozillaBrowserWindow::NotifyInvalidated(nsIWidget *aWidget, PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height)
{
	// try to match widget-window against ourselves to see if we need to update the texture
	// only works using native widgets (on Windows) at the moment - needs to be moved to platform agnostic code ASAP
	#ifdef WIN32

	// this is horrible beyond words but it seems to work...
	// nsToolkit tells us that a widget changed and we need to see if it's this instance
	// so we can emit an event that causes the parent app to update the browser texture
	nsIWidget* mainWidget;
	baseWindow->GetMainWidget(&mainWidget);

	HWND mainWindow = (HWND)mainWidget->GetNativeData(NS_NATIVE_WIDGET);
	HWND dirtyWindow = (HWND)aWidget->GetNativeData(NS_NATIVE_WIDGET);
	for(; dirtyWindow != 0; dirtyWindow = ::GetParent(dirtyWindow))
	{
		if(dirtyWindow == mainWindow)
		{
			for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
				(*i)->onPageChanged(this, x, y, width, height);
			break;
		};
	}

	// other platforms will always update - desperately inefficient but you'll see something.
	#else
		for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
			(*i)->onPageChanged(this, x, y, width, height);
	#endif

	return NS_OK;
} 

// End All Mozilla-Specific Definitions

void MozillaBrowserWindow::navigateTo(const std::string& url)
{
	webNav->LoadURI(reinterpret_cast<const PRUnichar*>(NS_ConvertUTF8toUTF16(url.c_str()).get()),
		nsIWebNavigation::LOAD_FLAGS_NONE, 0, 0, 0);
}

void MozillaBrowserWindow::navigateRefresh()
{
	webNav->Reload(nsIWebNavigation::LOAD_FLAGS_NONE);
}

void MozillaBrowserWindow::navigateStop()
{
	webNav->Stop(nsIWebNavigation::STOP_ALL);
}

void MozillaBrowserWindow::navigateForward()
{
	webNav->GoForward();
}

void MozillaBrowserWindow::navigateBack()
{
	webNav->GoBack();
}

bool MozillaBrowserWindow::canNavigateForward() const
{
	PRBool result = PR_FALSE;
	webNav->GetCanGoForward(&result);

	return result ? true : false;
}

bool MozillaBrowserWindow::canNavigateBack() const
{
	PRBool result = PR_FALSE;
	webNav->GetCanGoBack(&result);

	return result ? true : false;
}

std::string MozillaBrowserWindow::evaluateJS(const std::string& script)
{
	nsresult result;

	nsCOMPtr<nsIScriptGlobalObjectOwner> scriptOwner(do_GetInterface(webBrowser, &result));

	if(NS_FAILED(result))
		return "";

	nsIScriptContext* scriptContext = scriptOwner->GetScriptGlobalObject()->GetContext();

	PRBool undefined;
	nsString returnVal;
	result = scriptContext->EvaluateString(NS_ConvertUTF8toUTF16(script.c_str()), 0, 0, "", 1, 0, &returnVal, &undefined);

	if(NS_FAILED(result))
		return "";

	return std::string(NS_ConvertUTF16toUTF8(returnVal).get());
}

void MozillaBrowserWindow::focus()
{
	nsCOMPtr<nsIWebBrowserFocus> focus(do_GetInterface(webBrowser));
	focus->Activate();
}

void MozillaBrowserWindow::defocus()
{
	nsCOMPtr<nsIWebBrowserFocus> focus(do_GetInterface(webBrowser));
	focus->Deactivate();
}

void MozillaBrowserWindow::injectMouseMove(short x, short y)
{
	sendMozillaMouseEvent(NS_MOUSE_MOVE, x, y);
}

void MozillaBrowserWindow::injectMouseDown(short x, short y)
{
	sendMozillaMouseEvent(NS_MOUSE_LEFT_BUTTON_DOWN, x, y);
}

void MozillaBrowserWindow::injectMouseUp(short x, short y)
{
	sendMozillaMouseEvent(NS_MOUSE_LEFT_BUTTON_UP, x, y);
}

void MozillaBrowserWindow::injectKeyPress(short keyCode)
{
	sendMozillaKeyboardEvent(0, keyCode);
}

void MozillaBrowserWindow::injectScroll(short numLines)
{
	nsCOMPtr<nsIDOMWindow> window;
	nsresult result = webBrowser->GetContentDOMWindow(getter_AddRefs(window));

	if(!NS_FAILED(result))
		window->ScrollByLines(numLines);
}

void MozillaBrowserWindow::addListener(BrowserWindowListener* listener)
{
	for(ListenerIter i = listeners.begin(); i != listeners.end(); ++i)
		if(*i == listener)
			return;

	listeners.push_back(listener);
}

void MozillaBrowserWindow::removeListener(BrowserWindowListener* listener)
{
	for(ListenerIter i = listeners.begin(); i != listeners.end();)
	{
		if(*i == listener)
			i = listeners.erase(i);
		else
			++i;
	}
}

bool MozillaBrowserWindow::render()
{
	nsresult result;
	int x = 0;
	int y = 0;
	unsigned int width = surface.width;
	unsigned int height = surface.height;

	if(!okayToRender)
		return false;

	nsCOMPtr<nsIDocShell> docShell = do_GetInterface(webBrowser, &result);
	if(NS_FAILED(result))
		return false;

	nsCOMPtr<nsPresContext> presContext;
	result = docShell->GetPresContext(getter_AddRefs(presContext));
	if(!presContext)
		return false;

	nsIViewManager* viewManager = presContext->GetViewManager();
	if(!viewManager)
		return false;

	nsIView* view = 0;
	nsIScrollableView* scrollableView = 0;

	viewManager->GetRootScrollableView(&scrollableView);
	if(scrollableView)
		scrollableView->GetScrolledView(view);
	else
		viewManager->GetRootView(view);

	// get the rectangle we want to render in twips (this looks odd but takees care of scrolling too)
	nsRect rect = view->GetBounds() - view->GetPosition() - view->GetPosition();
	if(rect.IsEmpty())
		return false;

	float p2t = presContext->PixelsToTwips();
	rect.width = NSIntPixelsToTwips(width, p2t);
	rect.height = NSIntPixelsToTwips(height, p2t);

	// render the page
	nsCOMPtr<nsIRenderingContext> context;
	result = viewManager->RenderOffscreen(view, rect, PR_FALSE, PR_FALSE, bgColor, getter_AddRefs(context));
	if(NS_FAILED(result))
		return false;

	// retrieve the surface we rendered to
	nsIDrawingSurface* drawSurface = 0;
	context->GetDrawingSurface(&drawSurface);
	if(!drawSurface)
		return false;

	// lock the surface and retrieve a pointer to the rendered data and current row span
	PRUint8* data;
	PRInt32 widthBytes, rowPitch;
	// sometime rowspan ! width in pixels * bytes per pixel so save row span value and use in application
	result = drawSurface->Lock(x, y, width, height, reinterpret_cast<void**>(&data), &rowPitch, &widthBytes, NS_LOCK_SURFACE_READ_ONLY);
	if(NS_FAILED(result))
		return false;

	surface.resize(surface.width, surface.height, widthBytes / surface.width, rowPitch);

	// save the pixels
	memcpy(surface.buffer, data, height * rowPitch);

	// release and destroy the nsIDrawingSurface we rendered to
	drawSurface->Unlock();
	context->DestroyDrawingSurface(drawSurface);

	return true;
}

const RenderSurface& MozillaBrowserWindow::getRenderSurface() const
{
	return surface;
}

CaretInfo MozillaBrowserWindow::getCaretInfo() const
{
	nsresult result;
	nsCOMPtr<nsIWebBrowserFocus> focus = do_QueryInterface(webBrowser);

	nsCOMPtr<nsIDOMElement> focusedElement;
	result = focus->GetFocusedElement(getter_AddRefs(focusedElement));
	if(NS_FAILED(result))
		return CaretInfo();

	nsCOMPtr<nsIContent> focusedContent = do_QueryInterface(focusedElement);

	nsCOMPtr<nsIDOMWindow> domWindow;
	result = webBrowser->GetContentDOMWindow(getter_AddRefs(domWindow));
	if(NS_FAILED(result))
		return CaretInfo();

	nsCOMPtr<nsIDOMDocument> domDocument;
	result = domWindow->GetDocument(getter_AddRefs(domDocument));
	if(NS_FAILED(result))
		return CaretInfo();

	nsCOMPtr<nsIDocument> document = do_QueryInterface(domDocument, &result);
	if(NS_FAILED(result))
		return CaretInfo();

	nsIPresShell* presShell = document->GetShellAt(0);
	if(!presShell)
		return CaretInfo();

	nsCOMPtr<nsICaret> caret;
	presShell->GetCaret(getter_AddRefs(caret));

	nsIFrame* frame = 0;
	result = presShell->GetPrimaryFrameFor(focusedContent, &frame);
	if(NS_FAILED(result))
		return CaretInfo();

	nsCOMPtr<nsISelectionController> selCtrl;
	frame->GetSelectionController(presShell->GetPresContext(), getter_AddRefs(selCtrl));

	nsCOMPtr<nsISelection> selection;
	selCtrl->GetSelection(nsISelectionController::SELECTION_NORMAL, getter_AddRefs(selection));

	PRBool collapsed;
	nsRect coords;
	nsIView* caretView;
	result = caret->GetCaretCoordinates(nsICaret::eTopLevelWindowCoordinates, selection, &coords, &collapsed, &caretView);
	if(NS_FAILED(result))
		return CaretInfo();

	float twips2Pixls = presShell->GetPresContext()->TwipsToPixels();

	int x = NSTwipsToIntPixels(coords.x, twips2Pixls);
	int y = NSTwipsToIntPixels(coords.y, twips2Pixls);
	int height = NSTwipsToIntPixels(coords.height, twips2Pixls);

	return CaretInfo(true, x, y, height);
}

std::string MozillaBrowserWindow::getCurrentURL() const
{
	nsIURI* currentURI = 0;
	nsresult result = webNav->GetCurrentURI(&currentURI);
	if(NS_FAILED(result))
		return "";

	nsCAutoString urlString;
	currentURI->GetSpec(urlString);

	return std::string(urlString.get());
}

void MozillaBrowserWindow::resize(short width, short height)
{
	surface.resize(width, height, surface.depth, width * surface.depth);

#ifdef WIN32
	baseWindow->SetVisibility(PR_FALSE);
	baseWindow->SetPosition(0, 0);
	baseWindow->SetSize(width, height, false);
#endif
}

void MozillaBrowserWindow::setBackgroundColor(unsigned char red, unsigned char green, unsigned char blue)
{
	bgColor = NS_RGB(red, green, blue);
}

PRBool MozillaBrowserWindow::sendMozillaMouseEvent(PRInt16 eventIn, PRInt16 xPosIn, PRInt16 yPosIn)
{
	nsresult result;

	nsCOMPtr<nsIDocShell> docShell = do_GetInterface(webBrowser, &result);
	if(NS_FAILED(result))
		return false;

	nsCOMPtr<nsPresContext> presContext;
	result = docShell->GetPresContext(getter_AddRefs(presContext));
	if(NS_FAILED(result))
		return false;

	nsIViewManager* viewManager = presContext->GetViewManager();
	if(!viewManager)
		return false;

	nsIView* rootView = 0;
	result = viewManager->GetRootView(rootView);
	if(NS_FAILED(result))
		return false;

	nsCOMPtr<nsIWidget> widget = rootView->GetWidget();
	if(!widget)
		return false;

	nsMouseEvent mouseEvent(PR_TRUE, eventIn, widget, nsMouseEvent::eReal);
	mouseEvent.clickCount = 1;
	mouseEvent.isShift = 0;
	mouseEvent.isControl = 0;
	mouseEvent.isAlt = 0;
	mouseEvent.isMeta = 0;
	mouseEvent.widget = widget;
	mouseEvent.nativeMsg = nsnull;
	mouseEvent.point.x = xPosIn;
	mouseEvent.point.y = yPosIn;
	mouseEvent.refPoint.x = xPosIn;
	mouseEvent.refPoint.y = yPosIn;
	mouseEvent.flags = 0;

	nsEventStatus status;
	result = viewManager->DispatchEvent(&mouseEvent, &status);
	if(NS_FAILED(result))
		return false;

	return true;
}

PRBool MozillaBrowserWindow::sendMozillaKeyboardEvent(PRUint32 keyIn, PRUint32 ns_vk_code)
{
	nsresult result;

	nsCOMPtr<nsIDocShell> docShell = do_GetInterface(webBrowser, &result);
	if(NS_FAILED(result))
		return false;

	nsCOMPtr<nsPresContext> presContext;
	result = docShell->GetPresContext(getter_AddRefs(presContext));
	if(NS_FAILED(result))
		return false;

	nsIViewManager* viewManager = presContext->GetViewManager();
	if(!viewManager)
		return false;

	nsIView* rootView = 0;
	result = viewManager->GetRootView(rootView);
	if(NS_FAILED(result))
		return false;

	nsCOMPtr<nsIWidget> widget = rootView->GetWidget();
	if(!widget)
		return false;

	nsKeyEvent keyEvent(PR_TRUE, NS_KEY_PRESS, widget);
	keyEvent.keyCode = ns_vk_code;
	keyEvent.charCode = keyIn;
	keyEvent.isChar = PR_TRUE;
	keyEvent.isShift = 0;
	keyEvent.isControl = 0;
	keyEvent.isAlt = 0;
	keyEvent.isMeta = 0;
	keyEvent.widget = widget;
	keyEvent.nativeMsg = nsnull;
	keyEvent.point.x = 0;
	keyEvent.point.y = 0;
	keyEvent.refPoint.x = 0;
	keyEvent.refPoint.y = 0;
	keyEvent.flags = 0;

	nsEventStatus status;
	result = viewManager->DispatchEvent(&keyEvent, &status);
	if(NS_FAILED(result))
		return false;

	return true;
}

// #define required by this file for LibXUL/Mozilla code to avoid crashes in their debug code
#ifdef _DEBUG
	#ifdef WIN32
		#undef DEBUG
	#endif
#endif