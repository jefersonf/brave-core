/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.crypto_wallet.fragments;

import static org.chromium.chrome.browser.crypto_wallet.util.Utils.fromHexWei;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.base.Log;
import org.chromium.base.task.PostTask;
import org.chromium.brave_wallet.mojom.AccountInfo;
import org.chromium.brave_wallet.mojom.AssetPrice;
import org.chromium.brave_wallet.mojom.AssetPriceTimeframe;
import org.chromium.brave_wallet.mojom.AssetRatioController;
import org.chromium.brave_wallet.mojom.BraveWalletService;
import org.chromium.brave_wallet.mojom.ErcToken;
import org.chromium.brave_wallet.mojom.EthJsonRpcController;
import org.chromium.brave_wallet.mojom.KeyringController;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.crypto_wallet.ERCTokenRegistryFactory;
import org.chromium.chrome.browser.crypto_wallet.activities.BraveWalletActivity;
import org.chromium.chrome.browser.crypto_wallet.adapters.NetworkSpinnerAdapter;
import org.chromium.chrome.browser.crypto_wallet.adapters.WalletCoinAdapter;
import org.chromium.chrome.browser.crypto_wallet.listeners.OnWalletListItemClick;
import org.chromium.chrome.browser.crypto_wallet.model.WalletListItemModel;
import org.chromium.chrome.browser.crypto_wallet.util.SmoothLineChartEquallySpaced;
import org.chromium.chrome.browser.crypto_wallet.util.Utils;
import org.chromium.content_public.browser.UiThreadTaskTraits;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;

public class PortfolioFragment
        extends Fragment implements OnWalletListItemClick, AdapterView.OnItemSelectedListener {
    private static String TAG = "PortfolioFragment";
    private Spinner mSpinner;
    private TextView mBalance;

    public static PortfolioFragment newInstance() {
        return new PortfolioFragment();
    }

    private EthJsonRpcController getEthJsonRpcController() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletActivity) {
            return ((BraveWalletActivity) activity).getEthJsonRpcController();
        }

        return null;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
            @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_portfolio, container, false);

        view.setOnTouchListener(new View.OnTouchListener() {
            @Override
            @SuppressLint("ClickableViewAccessibility")
            public boolean onTouch(View v, MotionEvent event) {
                SmoothLineChartEquallySpaced chartES = view.findViewById(R.id.line_chart);
                if (chartES == null) {
                    return true;
                }
                if (event.getAction() == MotionEvent.ACTION_MOVE
                        || event.getAction() == MotionEvent.ACTION_DOWN) {
                    chartES.drawLine(event.getRawX(), null);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    chartES.drawLine(-1, null);
                }

                return true;
            }
        });

        mSpinner = view.findViewById(R.id.spinner);
        // class PortfolioFragment in fact is used both for Portfolio and Apps
        // screens, see CryptoFragmentPageAdapter.getItem.
        // Without post task with delay it happens that when 2nd instance is
        // created, then 1st instance onItemSelected is triggered with
        // position=0. That makes onItemSelected to switch network, but the
        // actual displayed value of spinner's item on 1st instance remains
        // unchanged.
        PostTask.postDelayedTask(UiThreadTaskTraits.DEFAULT,
                () -> { mSpinner.setOnItemSelectedListener(this); }, 500);

        mBalance = view.findViewById(R.id.balance);
        mBalance.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                updatePortfolio();
            }
        });

        // Creating adapter for spinner
        NetworkSpinnerAdapter dataAdapter = new NetworkSpinnerAdapter(getActivity(),
                Utils.getNetworksList(getActivity()), Utils.getNetworksAbbrevList(getActivity()));
        mSpinner.setAdapter(dataAdapter);

        return view;
    }

    private void updateNetwork() {
        EthJsonRpcController ethJsonRpcController = getEthJsonRpcController();
        if (ethJsonRpcController != null) {
            ethJsonRpcController.getChainId(chain_id -> {
                mSpinner.setSelection(getIndexOf(mSpinner, chain_id));
                updatePortfolio();
            });
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        updateNetwork();
    }

    private int getIndexOf(Spinner spinner, String chain_id) {
        String strNetwork = Utils.getNetworkText(getActivity(), chain_id).toString();
        for (int i = 0; i < spinner.getCount(); i++) {
            if (spinner.getItemAtPosition(i).toString().equalsIgnoreCase(strNetwork)) {
                return i;
            }
        }

        return 0;
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        String item = parent.getItemAtPosition(position).toString();
        EthJsonRpcController ethJsonRpcController = getEthJsonRpcController();
        if (ethJsonRpcController != null) {
            ethJsonRpcController.setNetwork(
                    Utils.getNetworkConst(getActivity(), item), (success) -> {
                        if (!success) {
                            Log.e(TAG, "Could not set network");
                        }
                    });
            updatePortfolio();
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> arg0) {}

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        assert getActivity() != null;

        SmoothLineChartEquallySpaced chartES = view.findViewById(R.id.line_chart);
        chartES.setColors(new int[] {0xFFF73A1C, 0xFFBF14A2, 0xFF6F4CD2});
        chartES.setData(new float[] {15, 21, 9, 21, 25, 35, 24, 28});

        Button editVisibleAssets = view.findViewById(R.id.edit_visible_assets);
        editVisibleAssets.setOnClickListener(v -> {
            String chainName = mSpinner.getSelectedItem().toString();
            String chainId = Utils.getNetworkConst(getActivity(), chainName);

            EditVisibleAssetsBottomSheetDialogFragment bottomSheetDialogFragment =
                    EditVisibleAssetsBottomSheetDialogFragment.newInstance(
                            WalletCoinAdapter.AdapterType.EDIT_VISIBLE_ASSETS_LIST);

            bottomSheetDialogFragment.setChainId(chainId);
            bottomSheetDialogFragment.setDismissListener(
                    new EditVisibleAssetsBottomSheetDialogFragment.DismissListener() {
                        @Override
                        public void onDismiss(Boolean isAssetsListChanged) {
                            if (isAssetsListChanged != null && isAssetsListChanged) {
                                updatePortfolio();
                            }
                        }
                    });

            bottomSheetDialogFragment.show(
                    getFragmentManager(), EditVisibleAssetsBottomSheetDialogFragment.TAG_FRAGMENT);
        });
    }

    private void setUpCoinList(
            HashMap<String, Double> perTokenCryptoSum, HashMap<String, Double> perTokenFiatSum) {
        View view = getView();
        assert view != null;
        BraveWalletService braveWalletService = getBraveWalletService();
        assert braveWalletService != null;
        String chainName = mSpinner.getSelectedItem().toString();
        String chainId = Utils.getNetworkConst(getActivity(), chainName);
        assert chainId != null && !chainId.isEmpty();
        braveWalletService.getUserAssets(chainId, (userAssets) -> {
            RecyclerView rvCoins = view.findViewById(R.id.rvCoins);
            WalletCoinAdapter walletCoinAdapter =
                    new WalletCoinAdapter(WalletCoinAdapter.AdapterType.VISIBLE_ASSETS_LIST);
            List<WalletListItemModel> walletListItemModelList = new ArrayList<>();
            String tokensPath = ERCTokenRegistryFactory.getInstance().getTokensIconsLocation();
            for (ErcToken userAsset : userAssets) {
                String currentAssetSymbol = userAsset.symbol.toLowerCase();
                Double fiatBalance = perTokenFiatSum.getOrDefault(currentAssetSymbol, 0.0d);
                String fiatBalanceString =
                        String.format(Locale.getDefault(), "$%,.2f", fiatBalance);
                Double cryptoBalance = perTokenFiatSum.getOrDefault(currentAssetSymbol, 0.0d);
                String cryptoBalanceString = String.format(
                        Locale.getDefault(), "%.4f %s", cryptoBalance, userAsset.symbol);

                WalletListItemModel walletListItemModel =
                        new WalletListItemModel(R.drawable.ic_eth, userAsset.name, userAsset.symbol,
                                // Amount in USD
                                fiatBalanceString,
                                // Amount in current crypto currency/token
                                cryptoBalanceString);
                walletListItemModel.setIconPath("file://" + tokensPath + "/" + userAsset.logo);
                walletListItemModelList.add(walletListItemModel);
            }

            walletCoinAdapter.setWalletListItemModelList(walletListItemModelList);
            walletCoinAdapter.setOnWalletListItemClick(PortfolioFragment.this);
            walletCoinAdapter.setWalletListItemType(Utils.ASSET_ITEM);
            rvCoins.setAdapter(walletCoinAdapter);
            rvCoins.setLayoutManager(new LinearLayoutManager(getActivity()));
        });
    }

    @Override
    public void onAssetClick() {
        Utils.openAssetDetailsActivity(getActivity());
    }

    private AssetRatioController getAssetRatioController() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletActivity) {
            return ((BraveWalletActivity) activity).getAssetRatioController();
        }

        return null;
    }

    private KeyringController getKeyringController() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletActivity) {
            return ((BraveWalletActivity) activity).getKeyringController();
        }

        return null;
    }

    BraveWalletService getBraveWalletService() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletActivity) {
            return ((BraveWalletActivity) activity).getBraveWalletService();
        } else {
            assert false;
        }

        return null;
    }

    // Helper to track multiple EthJsonRpcController.getBalance responses
    private class GetBalanceMultiResponse {
        private Runnable mWhenAllCompletedRunnable;
        private int mTotalElements;
        private int mCurrentElements;

        public GetBalanceMultiResponse(int totalElements) {
            synchronized (this) {
                mTotalElements = totalElements;
            }
        }

        // TODO(AlexeyBarabash): add runnable to handle the timeout case
        public void setWhenAllCompletedAction(Runnable whenAllCompletedRunnable) {
            synchronized (this) {
                assert this.mWhenAllCompletedRunnable == null;
                this.mWhenAllCompletedRunnable = whenAllCompletedRunnable;
                checkAndRunCompletedAction();
            }
        }

        public Runnable singleResponseComplete = new Runnable() {
            @Override
            public void run() {
                synchronized (this) {
                    mCurrentElements++;
                    assert mCurrentElements <= mTotalElements;
                    checkAndRunCompletedAction();
                }
            }
        };

        private void checkAndRunCompletedAction() {
            if (mCurrentElements == mTotalElements) {
                mWhenAllCompletedRunnable.run();
            }
        }
    }

    private class GetBalanceResponseBaseContext {
        public Boolean success;
        public String balance;
        public ErcToken userAsset;
        private Runnable mResponseCompleteCallback;

        public GetBalanceResponseBaseContext(Runnable responseCompleteCallback) {
            mResponseCompleteCallback = responseCompleteCallback;
        }

        public void callBase(Boolean success, String balance) {
            this.success = success;
            this.balance = balance;
            assert mResponseCompleteCallback != null;
            mResponseCompleteCallback.run();
        }
    };

    private class GetErc20TokenBalanceResponseContext extends GetBalanceResponseBaseContext
            implements EthJsonRpcController.GetErc20TokenBalanceResponse {
        public GetErc20TokenBalanceResponseContext(Runnable responseCompleteCallback) {
            super(responseCompleteCallback);
        }

        @Override
        public void call(Boolean success, String balance) {
            super.callBase(success, balance);
        }
    }

    private class GetBalanceResponseContext extends GetBalanceResponseBaseContext
            implements EthJsonRpcController.GetBalanceResponse {
        public GetBalanceResponseContext(Runnable responseCompleteCallback) {
            super(responseCompleteCallback);
        }

        @Override
        public void call(Boolean success, String balance) {
            super.callBase(success, balance);
        }
    }

    private void updatePortfolio() {
        // Get user assets
        // Get convertion prices
        // Get accounts
        // Get balance per account and asset contract address

        BraveWalletService braveWalletService = getBraveWalletService();
        assert braveWalletService != null;
        AssetRatioController assetRatioController = getAssetRatioController();
        assert assetRatioController != null;
        KeyringController keyringController = getKeyringController();
        assert keyringController != null;
        EthJsonRpcController rpcController = getEthJsonRpcController();
        assert rpcController != null;

        String chainName = mSpinner.getSelectedItem().toString();
        String chainId = Utils.getNetworkConst(getActivity(), chainName);
        assert chainId != null && !chainId.isEmpty();
        braveWalletService.getUserAssets(chainId, (userAssets) -> {
            String[] fromAssets = new String[userAssets.length];
            int i = 0;
            for (ErcToken userAsset : userAssets) {
                fromAssets[i] = userAsset.symbol.toLowerCase();
                ++i;
            }
            String[] toAssets = new String[] {"usd"};

            HashMap<String, Double> tokenToUsdRatios = new HashMap<String, Double>();

            // TODO(AlexeyBarabash):
            // Sometimes with certain tokens the price request gives 404 error
            // I need to query rate not all-in-one but per each token separately
            assetRatioController.getPrice(
                    fromAssets, toAssets, AssetPriceTimeframe.LIVE, (success, assetPrices) -> {
                        for (AssetPrice assetPrice : assetPrices) {
                            Double usdPerToken = 0.0d;
                            try {
                                usdPerToken = Double.parseDouble(assetPrice.price);
                            } catch (NullPointerException | NumberFormatException ex) {
                                Log.e(TAG, "Cannot parse " + assetPrice.price + ", " + ex);
                                return;
                            }

                            tokenToUsdRatios.put(assetPrice.fromAsset.toLowerCase(), usdPerToken);
                        }

                        keyringController.getDefaultKeyringInfo(keyringInfo -> {
                            if (keyringInfo != null) {
                                AccountInfo[] accountInfos = keyringInfo.accountInfos;

                                GetBalanceMultiResponse multiResponse = new GetBalanceMultiResponse(
                                        accountInfos.length * userAssets.length);

                                ArrayList<GetBalanceResponseBaseContext> contexts =
                                        new ArrayList<GetBalanceResponseBaseContext>();

                                // Tokens balances
                                for (AccountInfo accountInfo : accountInfos) {
                                    for (ErcToken userAsset : userAssets) {
                                        if (userAsset.contractAddress.isEmpty()) {
                                            GetBalanceResponseContext context =
                                                    new GetBalanceResponseContext(
                                                            multiResponse.singleResponseComplete);
                                            context.userAsset = userAsset;
                                            contexts.add(context);
                                            rpcController.getBalance(accountInfo.address, context);

                                        } else {
                                            GetErc20TokenBalanceResponseContext context =
                                                    new GetErc20TokenBalanceResponseContext(
                                                            multiResponse.singleResponseComplete);
                                            context.userAsset = userAsset;
                                            contexts.add(context);
                                            rpcController.getErc20TokenBalance(
                                                    userAsset.contractAddress, accountInfo.address,
                                                    context);
                                        }
                                    }
                                }

                                multiResponse.setWhenAllCompletedAction(() -> {
                                    Double totalFiatSum = 0.0d;
                                    HashMap<String, Double> perTokenFiatSum =
                                            new HashMap<String, Double>();

                                    HashMap<String, Double> perTokenCryptoSum =
                                            new HashMap<String, Double>();

                                    for (GetBalanceResponseBaseContext context : contexts) {
                                        Double usdPerThisToken = tokenToUsdRatios.getOrDefault(
                                                context.userAsset.symbol.toLowerCase(), 0.0d);

                                        Double thisBalanceCryptoPart = context.success
                                                ? fromHexWei(context.balance)
                                                : 0.0d;

                                        Double thisBalanceFiatPart =
                                                usdPerThisToken * thisBalanceCryptoPart;

                                        Double prevThisTokenCryptoSum =
                                                perTokenCryptoSum.getOrDefault(
                                                        context.userAsset.symbol.toLowerCase(),
                                                        0.0d);
                                        perTokenCryptoSum.put(
                                                context.userAsset.symbol.toLowerCase(),
                                                prevThisTokenCryptoSum + thisBalanceCryptoPart);

                                        Double prevThisTokenFiatSum = perTokenFiatSum.getOrDefault(
                                                context.userAsset.symbol.toLowerCase(), 0.0d);
                                        perTokenFiatSum.put(context.userAsset.symbol.toLowerCase(),
                                                prevThisTokenFiatSum + thisBalanceFiatPart);

                                        totalFiatSum += thisBalanceFiatPart;
                                    }

                                    final String fiatSumString =
                                            String.format("$%,.2f", totalFiatSum);
                                    PostTask.runOrPostTask(UiThreadTaskTraits.DEFAULT, () -> {
                                        mBalance.setText(fiatSumString);
                                        mBalance.invalidate();

                                        setUpCoinList(perTokenCryptoSum, perTokenFiatSum);
                                    });
                                });
                            }
                        });
                    });
        });
    }
}
