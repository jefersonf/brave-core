// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_BUBBLE_BUBBLE_CONTENTS_WRAPPER_H_
#define BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_BUBBLE_BUBBLE_CONTENTS_WRAPPER_H_

#include "chrome/browser/ui/views/bubble/bubble_contents_wrapper.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/base/window_open_disposition.h"
#include "url/gurl.h"

#define RenderViewHostChanged                                               \
  AddNewContents(content::WebContents* source,                              \
                 std::unique_ptr<content::WebContents> new_contents,        \
                 const GURL& target_url, WindowOpenDisposition disposition, \
                 const gfx::Rect& initial_rect, bool user_gesture,          \
                 bool* was_blocked) override;                               \
  void RenderViewHostChanged
#include "../../../../../../../chrome/browser/ui/views/bubble/bubble_contents_wrapper.h"
#undef RenderViewHostChanged

#endif  // BRAVE_CHROMIUM_SRC_CHROME_BROWSER_UI_VIEWS_BUBBLE_BUBBLE_CONTENTS_WRAPPER_H_
