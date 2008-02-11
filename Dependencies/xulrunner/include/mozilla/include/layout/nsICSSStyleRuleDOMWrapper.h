/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is nsICSSStyleRuleDOMWrapper.h.
 *
 * The Initial Developer of the Original Code is L. David Baron.
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   L. David Baron <dbaron@dbaron.org> (original author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef nsICSSStyleRuleDOMWrapper_h_
#define nsICSSStyleRuleDOMWrapper_h_

#include "nsIDOMCSSStyleRule.h"

// IID for the nsICSSStyleRuleDOMWrapper interface
// {476a4290-1194-4099-8f2d-a1ccc9bdd676}
#define NS_ICSS_STYLE_RULE_DOM_WRAPPER_IID \
{0x476a4290, 0x1194, 0x4099, {0x8f, 0x2d, 0xa1, 0xcc, 0xc9, 0xbd, 0xd6, 0x76}}

class nsICSSStyleRuleDOMWrapper : public nsIDOMCSSStyleRule {
public:
  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ICSS_STYLE_RULE_DOM_WRAPPER_IID)

  NS_IMETHOD GetCSSStyleRule(nsICSSStyleRule** aResult) = 0;
};

#endif /* !defined(nsICSSStyleRuleDOMWrapper_h_) */
