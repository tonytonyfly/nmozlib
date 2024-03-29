/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM c:/mozilla/dom/public/idl/events/nsIDOMNSEventTarget.idl
 */

#ifndef __gen_nsIDOMNSEventTarget_h__
#define __gen_nsIDOMNSEventTarget_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMNSEventTarget */
#define NS_IDOMNSEVENTTARGET_IID_STR "6cbbbf64-212f-4ef8-9ad4-7240dbb8d6ac"

#define NS_IDOMNSEVENTTARGET_IID \
  {0x6cbbbf64, 0x212f, 0x4ef8, \
    { 0x9a, 0xd4, 0x72, 0x40, 0xdb, 0xb8, 0xd6, 0xac }}

/**
 * The nsIDOMNSEventTarget interface is an extension to the standard
 * nsIDOMEventTarget interface, implemented by all event targets in
 * the Document Object Model.
 */
class NS_NO_VTABLE nsIDOMNSEventTarget : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDOMNSEVENTTARGET_IID)

  /**
   * This method is the same as the addEventListener() method defined
   * in nsIDOMEventTarget, but it takes one additional argument which
   * lets callers control whether or not they want to receive
   * untrusted events (synthetic events generated by untrusted code)
   *
   * @param   type See the type argument to the same method in
   *               nsIDOMEventTarget.
   * @param   listener See the listener argument to the same method in
   *                   nsIDOMEventTarget.
   * @param   useCapture See the listener argument to the same method in
   *                     nsIDOMEventTarget.
   * @param   wantsUntrusted If false, the listener will not receive any
   *                         untrusted events (see above), if true, the
   *                         listener will receive events whether or not
   *                         they're trusted
   */
  /* void addEventListener (in DOMString type, in nsIDOMEventListener listener, in boolean useCapture, in boolean wantsUntrusted); */
  NS_IMETHOD AddEventListener(const nsAString & type, nsIDOMEventListener *listener, PRBool useCapture, PRBool wantsUntrusted) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNSEVENTTARGET \
  NS_IMETHOD AddEventListener(const nsAString & type, nsIDOMEventListener *listener, PRBool useCapture, PRBool wantsUntrusted); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNSEVENTTARGET(_to) \
  NS_IMETHOD AddEventListener(const nsAString & type, nsIDOMEventListener *listener, PRBool useCapture, PRBool wantsUntrusted) { return _to AddEventListener(type, listener, useCapture, wantsUntrusted); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNSEVENTTARGET(_to) \
  NS_IMETHOD AddEventListener(const nsAString & type, nsIDOMEventListener *listener, PRBool useCapture, PRBool wantsUntrusted) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddEventListener(type, listener, useCapture, wantsUntrusted); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNSEventTarget : public nsIDOMNSEventTarget
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNSEVENTTARGET

  nsDOMNSEventTarget();

private:
  ~nsDOMNSEventTarget();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNSEventTarget, nsIDOMNSEventTarget)

nsDOMNSEventTarget::nsDOMNSEventTarget()
{
  /* member initializers and constructor code */
}

nsDOMNSEventTarget::~nsDOMNSEventTarget()
{
  /* destructor code */
}

/* void addEventListener (in DOMString type, in nsIDOMEventListener listener, in boolean useCapture, in boolean wantsUntrusted); */
NS_IMETHODIMP nsDOMNSEventTarget::AddEventListener(const nsAString & type, nsIDOMEventListener *listener, PRBool useCapture, PRBool wantsUntrusted)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMNSEventTarget_h__ */
