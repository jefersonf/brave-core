/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import {loadTimeData} from '../i18n_setup.js';

(function() {
'use strict';

/**
 * 'settings-brave-rewards-page' is the settings page containing settings for
 * Brave Rewards.
 */
Polymer({
  is: 'settings-brave-rewards-page',

  properties: {
    maxAdsToDisplay_: Array,
    adsSubdivisionTargetingCodes_: Array,
    autoContributeMonthlyLimit_: Array,
    autoContributeMinVisitTimeOptions_: Array,
    autoContributeMinVisitsOptions_: Array
  },

  /** @private {?settings.BraveRewardsBrowserProxy} */
  browserProxy_: null,

  /** @override */
  created: function() {
    this.browserProxy_ = settings.BraveRewardsBrowserProxyImpl.getInstance();
  },

  /** @override */
  ready: function() {
    this.openRewardsPanel_ = this.openRewardsPanel_.bind(this)
    this.maxAdsToDisplay_ = [
      { name: loadTimeData.getString('braveRewardsDefaultItem'), value: -1 },
      { name: loadTimeData.getString('braveRewardsMaxAdsPerHour0'), value: 0 },
      { name: loadTimeData.getString('braveRewardsMaxAdsPerHour1'), value: 1 },
      { name: loadTimeData.getString('braveRewardsMaxAdsPerHour2'), value: 2 },
      { name: loadTimeData.getString('braveRewardsMaxAdsPerHour3'), value: 3 },
      { name: loadTimeData.getString('braveRewardsMaxAdsPerHour4'), value: 4 },
      { name: loadTimeData.getString('braveRewardsMaxAdsPerHour5'), value: 5 },
      { name: loadTimeData.getString('braveRewardsMaxAdsPerHour10'), value: 10 }
    ];
    this.adsSubdivisionTargetingCodes_ = [
      { name: loadTimeData.getString('braveRewardsAutoDetectedItem'), value: 'AUTO' },
      { name: loadTimeData.getString('braveRewardsDisabledItem'), value: 'DISABLED' },
      { name: 'Alabama', value: 'US-AL' },
      { name: 'Alaska', value: 'US-AK' },
      { name: 'Arizona', value: 'US-AZ' },
      { name: 'Arkansas', value: 'US-AR' },
      { name: 'California', value: 'US-CA' },
      { name: 'Colorado', value: 'US-CO' },
      { name: 'Connecticut', value: 'US-CT' },
      { name: 'Delaware', value: 'US-DE' },
      { name: 'Florida', value: 'US-FL' },
      { name: 'Georgia', value: 'US-GA' },
      { name: 'Hawaii', value: 'US-HI' },
      { name: 'Idaho', value: 'US-ID' },
      { name: 'Illinois', value: 'US-IL' },
      { name: 'Indiana', value: 'US-IN' },
      { name: 'Iowa', value: 'US-IA' },
      { name: 'Kansas', value: 'US-KS' },
      { name: 'Kentucky', value: 'US-KY' },
      { name: 'Louisiana', value: 'US-LA' },
      { name: 'Maine', value: 'US-ME' },
      { name: 'Maryland', value: 'US-MD' },
      { name: 'Massachusetts', value: 'US-MA' },
      { name: 'Michigan', value: 'US-MI' },
      { name: 'Minnesota', value: 'US-MN' },
      { name: 'Mississippi', value: 'US-MS' },
      { name: 'Missouri', value: 'US-MO' },
      { name: 'Montana', value: 'US-MT' },
      { name: 'Nebraska', value: 'US-NE' },
      { name: 'Nevada', value: 'US-NV' },
      { name: 'New Hampshire', value: 'US-NH' },
      { name: 'New Jersey', value: 'US-NJ' },
      { name: 'New Mexico', value: 'US-NM' },
      { name: 'New York', value: 'US-NY' },
      { name: 'North Carolina', value: 'US-NC' },
      { name: 'North Dakota', value: 'US-ND' },
      { name: 'Ohio', value: 'US-OH' },
      { name: 'Oklahoma', value: 'US-OK' },
      { name: 'Oregon', value: 'US-OR' },
      { name: 'Pennsylvania', value: 'US-PA' },
      { name: 'Rhode Island', value: 'US-RI' },
      { name: 'South Carolina', value: 'US-SC' },
      { name: 'South Dakota', value: 'US-SD' },
      { name: 'Tennessee', value: 'US-TN' },
      { name: 'Texas', value: 'US-TX' },
      { name: 'Utah', value: 'US-UT' },
      { name: 'Vermont', value: 'US-VT' },
      { name: 'Virginia', value: 'US-VA' },
      { name: 'Washington', value: 'US-WA' },
      { name: 'West Virginia', value: 'US-WV' },
      { name: 'Wisconsin', value: 'US-WI' },
      { name: 'Wyoming', value: 'US-WY' }
    ];
    this.autoContributeMinVisitTimeOptions_ = [
      { name: loadTimeData.getString('braveRewardsAutoContributeMinVisitTime5'), value: 5 },
      { name: loadTimeData.getString('braveRewardsAutoContributeMinVisitTime8'), value: 8 },
      { name: loadTimeData.getString('braveRewardsAutoContributeMinVisitTime60'), value: 60 }
    ];
    this.autoContributeMinVisitsOptions_ = [
      { name: loadTimeData.getString('braveRewardsAutoContributeMinVisits1'), value: 1 },
      { name: loadTimeData.getString('braveRewardsAutoContributeMinVisits5'), value: 5 },
      { name: loadTimeData.getString('braveRewardsAutoContributeMinVisits10'), value: 10 }
    ];
    this.browserProxy_.isRewardsEnabled().then((enabled) => {
      if (enabled) {
        this.populateAutoContributeAmountDropdown_();
      }
    });
  },

  /** @private */
  populateAutoContributeAmountDropdown_: function () {
    this.browserProxy_.getRewardsParameters().then((params) => {
      let autoContributeChoices = [
        { name: loadTimeData.getString('braveRewardsDefaultItem'), value: 0 }
      ];
      params.autoContributeChoices.forEach((element) => {
        autoContributeChoices.push({
          name: `${loadTimeData.getString('braveRewardsContributionUpTo')} ${element.toFixed(3)} BAT`,
          value: element
        });
      });
      this.autoContributeMonthlyLimit_ = autoContributeChoices;
    });
  },

  shouldAllowAdsSubdivisionTargeting_: function() {
    return navigator.language === 'en-US';
  },

  isRewardsEnabled_: function(rewardsEnabled, adsEnabled) {
    return rewardsEnabled || adsEnabled;
  },

  openRewardsPanel_: function () {
    chrome.braveRewards.openBrowserActionUI('brave_rewards_panel.html');
    this.populateAutoContributeAmountDropdown_();
  }
});
})();
