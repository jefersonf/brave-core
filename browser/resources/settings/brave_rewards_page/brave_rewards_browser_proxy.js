/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import {addSingletonGetter} from 'chrome://resources/js/cr.m.js';

/** @interface */
export class BraveRewardsBrowserProxy {
  isRewardsEnabled() {}
  getRewardsParameters() {}
}

/**
 * @implements {settings.BraveRewardsBrowserProxy}
 */
export class BraveRewardsBrowserProxyImpl {
  /** @override */
  isRewardsEnabled () {
    return new Promise((resolve) => chrome.braveRewards.shouldShowOnboarding(
      (showOnboarding) => { resolve(!showOnboarding) }))
  }
  /** @override */
  getRewardsParameters () {
    return new Promise((resolve) => chrome.braveRewards.getRewardsParameters(
      (parameters) => { resolve(parameters) }))
  }
}

addSingletonGetter(BraveRewardsBrowserProxyImpl)
