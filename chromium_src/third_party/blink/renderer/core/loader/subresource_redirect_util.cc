/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/core/loader/subresource_redirect_util.h"

#define ShouldDisableCSPCheckForLitePageSubresourceRedirectOrigin \
  ShouldDisableCSPCheckForLitePageSubresourceRedirectOrigin_ChromiumImpl

#include "../../../../../../../third_party/blink/renderer/core/loader/subresource_redirect_util.cc"
#undef ShouldDisableCSPCheckForLitePageSubresourceRedirectOrigin

namespace blink {

bool ShouldDisableCSPCheckForLitePageSubresourceRedirectOrigin(
    scoped_refptr<SecurityOrigin> litepage_subresource_redirect_origin,
    mojom::blink::RequestContextType request_context,
    ResourceRequest::RedirectStatus redirect_status,
    const KURL& url) {
  String host = url.Host();
  if (host == "pcdn.brave.com" || host == "pcdn.bravesoftware.com" ||
      host == "pcdn.brave.software") {
    return true;
  }
  return ShouldDisableCSPCheckForLitePageSubresourceRedirectOrigin_ChromiumImpl(
      litepage_subresource_redirect_origin, request_context, redirect_status,
      url);
}

}  // namespace blink
