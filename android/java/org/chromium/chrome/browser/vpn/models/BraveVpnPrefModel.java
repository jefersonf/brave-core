/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.vpn.models;

public class BraveVpnPrefModel {
    private String mPurchaseToken;
    private String mProductId;
    private String mSubscriberCredential;
    private String mHostname;

    public void setPurchaseToken(String purchaseToken) {
        mPurchaseToken = purchaseToken;
    }

    public void setProductId(String productId) {
        mProductId = productId;
    }

    public void setSubscriberCredential(String subscriberCredential) {
        mSubscriberCredential = subscriberCredential;
    }

    public void setHostname(String hostname) {
        mHostname = hostname;
    }

    public String getPurchaseToken() {
        return mPurchaseToken;
    }

    public String getProductId() {
        return mProductId;
    }

    public String getSubscriberCredential() {
        return mSubscriberCredential;
    }

    public String getHostname() {
        return mHostname;
    }
}
