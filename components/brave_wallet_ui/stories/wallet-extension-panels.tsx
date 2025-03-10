import * as React from 'react'

// Components
import {
  ConnectWithSite,
  ConnectedPanel,
  Panel,
  WelcomePanel,
  SignPanel,
  AllowAddNetworkPanel,
  ConfirmTransactionPanel,
  ConnectHardwareWalletPanel,
  SitePermissions
} from '../components/extension'
import { AppList } from '../components/shared'
import {
  Send,
  Buy,
  SelectAsset,
  SelectNetwork,
  SelectAccount
} from '../components/buy-send-swap'
import {
  WalletAccountType,
  PanelTypes,
  AppObjectType,
  AppsListType,
  AccountAssetOptionType,
  BuySendSwapViewTypes,
  EthereumChain,
  TransactionInfo,
  TransactionType
} from '../constants/types'
import {
  UpdateUnapprovedTransactionGasFieldsType,
  UpdateUnapprovedTransactionSpendAllowanceType
} from '../common/constants/action_types'
import { AppsList } from '../options/apps-list-options'
import { WyreAccountAssetOptions } from '../options/wyre-asset-options'
import { filterAppList } from '../utils/filter-app-list'
import { BuyAssetUrl } from '../utils/buy-asset-url'
import LockPanel from '../components/extension/lock-panel'
import {
  StyledExtensionWrapperLonger,
  StyledExtensionWrapper,
  ScrollContainer,
  SelectContainer
} from './style'
import { mockNetworks } from './mock-data/mock-networks'
import { AccountAssetOptions, NewAssetOptions } from '../options/asset-options'
import { PanelTitles } from '../options/panel-titles'
import './locale'
export default {
  title: 'Wallet/Extension/Panels',
  parameters: {
    layout: 'centered'
  },
  argTypes: {
    locked: { control: { type: 'boolean', lock: false } }
  }
}

const accounts: WalletAccountType[] = [
  {
    id: '1',
    name: 'Account 1',
    address: '0x7d66c9ddAED3115d93Bd1790332f3Cd06Cf52B14',
    balance: '0.31178',
    asset: 'eth',
    fiatBalance: '0',
    accountType: 'Primary',
    tokens: []
  },
  {
    id: '2',
    name: 'Account 2',
    address: '0x73A29A1da97149722eB09c526E4eAd698895bDCf',
    balance: '0.31178',
    asset: 'eth',
    fiatBalance: '0',
    accountType: 'Primary',
    tokens: []
  },
  {
    id: '3',
    name: 'Account 3',
    address: '0x3f29A1da97149722eB09c526E4eAd698895b426',
    balance: '0.31178',
    asset: 'eth',
    fiatBalance: '0',
    accountType: 'Primary',
    tokens: []
  }
]

export const _ConfirmTransaction = () => {

  const transactionInfo: TransactionInfo = {
    fromAddress: '0x7d66c9ddAED3115d93Bd1790332f3Cd06Cf52B14',
    id: '465a4d6646-kjlwf665',
    txArgs: ['0x0d8775f648430679a709e98d2b0cb6250d2887ef', '0x15ddf09c97b0000'],
    txData: {
      baseData: {
        nonce: '0x1',
        gasPrice: '150',
        gasLimit: '21000',
        to: '2',
        value: '0x15ddf09c97b0000',
        data: new Uint8Array(24)
      },
      chainId: '0x0',
      maxPriorityFeePerGas: '',
      maxFeePerGas: ''
    },
    txHash: '0xab834bab0000000000000000000000007be8076f4ea4a4ad08075c2508e481d6c946d12b00000000000000000000000073a29a1da971497',
    txStatus: 0,
    txParams: ['address', 'ammount'],
    txType: TransactionType.ERC20Transfer,
    createdTime: { microseconds: 0 },
    submittedTime: { microseconds: 0 },
    confirmedTime: { microseconds: 0 }
  }

  const onConfirmTransaction = () => {
    alert('Confirmed Transaction')
  }

  const onRejectTransaction = () => {
    alert('Rejected Transaction')
  }

  const onRejectAllTransactions = () => {
    alert('Rejected All Transaction')
  }
  const onQueueNextTransction = () => {
    alert('Will queue next transaction in line')
  }

  const refreshGasEstimates = () => {
    // do nothing
  }

  const updateUnapprovedTransactionGasFields = (payload: UpdateUnapprovedTransactionGasFieldsType) => {
    alert('Updated gas fields')
  }

  const updateUnapprovedTransactionSpendAllowance = (payload: UpdateUnapprovedTransactionSpendAllowanceType) => {
    alert('Updated spending allowance')
  }

  const getERC20Allowance = (recipient: string, sender: string, approvalTarget: string) => {
    return Promise.resolve('0x15ddf09c97b0000')
  }

  const transactionSpotPrices = [
    {
      fromAsset: 'ETH',
      toAsset: 'USD',
      price: '3300',
      assetTimeframeChange: ''
    },
    {
      fromAsset: 'BAT',
      toAsset: 'USD',
      price: '0.85',
      assetTimeframeChange: ''
    }
  ]

  return (
    <StyledExtensionWrapperLonger>
      <ConfirmTransactionPanel
        selectedNetwork={mockNetworks[0]}
        onQueueNextTransction={onQueueNextTransction}
        onRejectAllTransactions={onRejectAllTransactions}
        transactionQueueNumber={0}
        transactionsQueueLength={0}
        onConfirm={onConfirmTransaction}
        onReject={onRejectTransaction}
        accounts={accounts}
        transactionInfo={transactionInfo}
        visibleTokens={NewAssetOptions}
        transactionSpotPrices={transactionSpotPrices}
        refreshGasEstimates={refreshGasEstimates}
        updateUnapprovedTransactionGasFields={updateUnapprovedTransactionGasFields}
        updateUnapprovedTransactionSpendAllowance={updateUnapprovedTransactionSpendAllowance}
        getERC20Allowance={getERC20Allowance}
      />
    </StyledExtensionWrapperLonger>
  )
}

_ConfirmTransaction.story = {
  name: 'Confirm Transaction'
}

export const _AllowAddNetwork = () => {

  const onApprove = () => {
    alert('Approved Adding Network')
  }

  const onCancel = () => {
    alert('Canceled Adding Network')
  }

  const onLearnMore = () => {
    alert('Will nav to Learn More')
  }

  return (
    <StyledExtensionWrapperLonger>
      <AllowAddNetworkPanel
        onApprove={onApprove}
        onCancel={onCancel}
        networkPayload={mockNetworks[0]}
        onLearnMore={onLearnMore}
      />
    </StyledExtensionWrapperLonger>
  )
}

_AllowAddNetwork.story = {
  name: 'Allow Add Network'
}

export const _SignData = () => {

  const onSign = () => {
    alert('Signed Data')
  }

  const onCancel = () => {
    alert('Canceled Signing Data')
  }

  return (
    <StyledExtensionWrapperLonger>
      <SignPanel
        selectedAccount={accounts[0]}
        selectedNetwork={mockNetworks[0]}
        message='To avoid digital cat burglars, sign below to authenticate with CryptoKitties.'
        onCancel={onCancel}
        onSign={onSign}
        showWarning={true}
      />
    </StyledExtensionWrapperLonger>
  )
}

_SignData.story = {
  name: 'Sign Transaction'
}

export const _ConnectWithSite = () => {
  const [selectedAccounts, setSelectedAccounts] = React.useState<WalletAccountType[]>([
    accounts[0]
  ])
  const [readyToConnect, setReadyToConnect] = React.useState<boolean>(false)
  const url = 'https://app.uniswap.org'
  const selectAccount = (account: WalletAccountType) => {
    const newList = [...selectedAccounts, account]
    setSelectedAccounts(newList)
  }
  const removeAccount = (account: WalletAccountType) => {
    const newList = selectedAccounts.filter(
      (accounts) => accounts.id !== account.id
    )
    setSelectedAccounts(newList)
  }
  const onSubmit = () => {
    alert(`Connecting to ${url} using: ${JSON.stringify(selectedAccounts)}`)
  }
  const primaryAction = () => {
    if (!readyToConnect) {
      setReadyToConnect(true)
    } else {
      onSubmit()
    }
  }
  const secondaryAction = () => {
    if (readyToConnect) {
      setReadyToConnect(false)
    } else {
      alert('You Clicked The Cancel Button!')
    }
  }
  return (
    <StyledExtensionWrapperLonger>
      <ConnectWithSite
        siteURL={url}
        isReady={readyToConnect}
        accounts={accounts}
        primaryAction={primaryAction}
        secondaryAction={secondaryAction}
        selectAccount={selectAccount}
        removeAccount={removeAccount}
        selectedAccounts={selectedAccounts}
      />
    </StyledExtensionWrapperLonger>
  )
}

_ConnectWithSite.story = {
  name: 'Connect With Site'
}

export const _ConnectedPanel = (args: { locked: boolean }) => {
  const { locked } = args
  const [inputValue, setInputValue] = React.useState<string>('')
  const [walletLocked, setWalletLocked] = React.useState<boolean>(locked)
  const [selectedPanel, setSelectedPanel] = React.useState<PanelTypes>('main')
  const [panelTitle, setPanelTitle] = React.useState<string>('main')
  const [selectedAccount, setSelectedAccount] = React.useState<WalletAccountType>(
    accounts[0]
  )
  const [favoriteApps, setFavoriteApps] = React.useState<AppObjectType[]>([
    AppsList()[0].appList[0]
  ])
  const [filteredAppsList, setFilteredAppsList] = React.useState<AppsListType[]>(AppsList())
  const [hasPasswordError, setHasPasswordError] = React.useState<boolean>(false)
  const [selectedNetwork, setSelectedNetwork] = React.useState<EthereumChain>(mockNetworks[0])
  const [selectedWyreAsset, setSelectedWyreAsset] = React.useState<AccountAssetOptionType>(WyreAccountAssetOptions[0])
  const [selectedAsset, setSelectedAsset] = React.useState<AccountAssetOptionType>(AccountAssetOptions[0])
  const [showSelectAsset, setShowSelectAsset] = React.useState<boolean>(false)
  const [toAddress, setToAddress] = React.useState('')
  const [fromAmount, setFromAmount] = React.useState('')
  const [buyAmount, setBuyAmount] = React.useState('')

  const onSetBuyAmount = (value: string) => {
    setBuyAmount(value)
  }

  const onSubmitBuy = () => {
    const url = BuyAssetUrl(mockNetworks[0].chainId, selectedWyreAsset, selectedAccount, buyAmount)
    if (url) {
      window.open(url, '_blank')
    }
  }

  const onChangeSendView = (view: BuySendSwapViewTypes) => {
    if (view === 'assets') {
      setShowSelectAsset(true)
    }
  }

  const onBack = () => {
    setSelectedPanel('main')
  }

  const onSelectNetwork = (network: EthereumChain) => () => {
    setSelectedNetwork(network)
    setSelectedPanel('main')
  }

  const onSelectAccount = (account: WalletAccountType) => () => {
    setSelectedAccount(account)
    setSelectedPanel('main')
  }

  const onHideSelectAsset = () => {
    setShowSelectAsset(false)
  }

  const onSelectAsset = (asset: AccountAssetOptionType) => () => {
    if (selectedPanel === 'buy') {
      setSelectedWyreAsset(asset)
    } else {
      setSelectedAsset(asset)
    }
    setShowSelectAsset(false)
  }

  const onInputChange = (value: string, name: string) => {
    if (name === 'address') {
      setToAddress(value)
    } else {
      setFromAmount(value)
    }
  }

  const getTitle = (path: PanelTypes) => {
    const title = PanelTitles().find((title) => path === title.id)
    setPanelTitle(title ? title.title : '')
  }

  const navigateTo = (path: PanelTypes) => {
    if (path === 'expanded') {
      alert('This will expand to main wallet!')
    } else {
      setSelectedPanel(path)
    }
    getTitle(path)
  }

  const browseMore = () => {
    alert('Will expand to view more!')
  }

  const addToFavorites = (app: AppObjectType) => {
    const newList = [...favoriteApps, app]
    setFavoriteApps(newList)
  }
  const removeFromFavorites = (app: AppObjectType) => {
    const newList = favoriteApps.filter(
      (fav) => fav.name !== app.name
    )
    setFavoriteApps(newList)
  }

  const filterList = (event: any) => {
    filterAppList(event, AppsList(), setFilteredAppsList)
  }

  const unlockWallet = () => {
    if (inputValue !== 'password') {
      setHasPasswordError(true)
    } else {
      setWalletLocked(false)
    }
  }

  const onLockWallet = () => {
    setWalletLocked(true)
  }

  const onSelectPresetAmount = (percent: number) => {
    const amount = Number(selectedAccount.balance) * percent
    setFromAmount(amount.toString())
  }

  const handlePasswordChanged = (value: string) => {
    setHasPasswordError(false)
    setInputValue(value)
  }

  const onSubmitSend = () => {
    alert('Will submit Send')
  }

  const onOpenSettings = () => {
    alert('Will go to Wallet Settings')
  }

  const onRestore = () => {
    alert('Will navigate to full wallet restore page')
  }

  const onDisconnectFromOrigin = (origin: string, address: string) => {
    console.log(`Will disconnect ${address} from ${origin}`)
  }

  const onAddAccount = () => {
    console.log('Will Expand to the Accounts Page')
  }

  return (
    <StyledExtensionWrapper>
      {walletLocked ? (
        <LockPanel
          hasPasswordError={hasPasswordError}
          onSubmit={unlockWallet}
          disabled={inputValue === ''}
          onPasswordChanged={handlePasswordChanged}
          onClickRestore={onRestore}
        />
      ) : (
        <>
          {selectedPanel === 'main' ? (
            <ConnectedPanel
              selectedNetwork={selectedNetwork}
              selectedAccount={selectedAccount}
              isConnected={true}
              navAction={navigateTo}
              onLockWallet={onLockWallet}
              onOpenSettings={onOpenSettings}
              activeOrigin=''
            />
          ) : (
            <>
              {showSelectAsset &&
                <SelectContainer>
                  <SelectAsset
                    assets={AccountAssetOptions}
                    onSelectAsset={onSelectAsset}
                    onBack={onHideSelectAsset}
                  />
                </SelectContainer>
              }
              {selectedPanel === 'accounts' &&
                <SelectContainer>
                  <SelectAccount
                    accounts={accounts}
                    onBack={onBack}
                    onSelectAccount={onSelectAccount}
                    onAddAccount={onAddAccount}
                    hasAddButton={true}
                  />
                </SelectContainer>
              }
              {selectedPanel === 'networks' &&
                <SelectContainer>
                  <SelectNetwork
                    networks={mockNetworks}
                    onBack={onBack}
                    onSelectNetwork={onSelectNetwork}
                  />
                </SelectContainer>
              }
              {!showSelectAsset && selectedPanel !== 'networks' && selectedPanel !== 'accounts' &&
                <Panel
                  navAction={navigateTo}
                  title={panelTitle}
                  useSearch={selectedPanel === 'apps'}
                  searchAction={selectedPanel === 'apps' ? filterList : undefined}
                >
                  <ScrollContainer>
                    {selectedPanel === 'apps' &&
                      <AppList
                        list={filteredAppsList}
                        favApps={favoriteApps}
                        addToFav={addToFavorites}
                        removeFromFav={removeFromFavorites}
                        action={browseMore}
                      />
                    }
                    {selectedPanel === 'send' &&
                      <Send
                        onChangeSendView={onChangeSendView}
                        onInputChange={onInputChange}
                        onSelectPresetAmount={onSelectPresetAmount}
                        onSubmit={onSubmitSend}
                        addressError=''
                        selectedAsset={selectedAsset}
                        selectedAssetAmount={fromAmount}
                        selectedAssetBalance={selectedAccount.balance.toString()}
                        toAddressOrUrl={toAddress}
                        toAddress={toAddress}
                      />
                    }
                    {selectedPanel === 'buy' &&
                      <Buy
                        onChangeBuyView={onChangeSendView}
                        onInputChange={onSetBuyAmount}
                        onSubmit={onSubmitBuy}
                        selectedAsset={selectedWyreAsset}
                        buyAmount={buyAmount}
                        selectedNetwork={selectedNetwork}
                        networkList={[]}
                      />
                    }
                    {selectedPanel === 'sitePermissions' &&
                      <SitePermissions
                        siteURL='https://app.uniswap.org'
                        onDisconnect={onDisconnectFromOrigin}
                        connectedAccounts={accounts}
                      />
                    }
                  </ScrollContainer>
                </Panel>
              }
            </>
          )}
        </>
      )}
    </StyledExtensionWrapper>
  )
}

_ConnectedPanel.args = {
  locked: false
}

_ConnectedPanel.story = {
  name: 'Connected With Site'
}

export const _SetupWallet = () => {

  const onRestore = () => {
    alert('Will navigate to full wallet restore page')
  }

  const onSetup = () => {
    alert('Will navigate to full wallet onboarding page')
  }

  return (
    <StyledExtensionWrapperLonger>
      <WelcomePanel onRestore={onRestore} onSetup={onSetup} />
    </StyledExtensionWrapperLonger>
  )
}

_SetupWallet.story = {
  name: 'Setup New Wallet'
}

export const _ConnectHardwareWallet = () => {

  const onCancel = () => {
    // Doesn't do anything in storybook
  }

  return (
    <StyledExtensionWrapper>
      <ConnectHardwareWalletPanel
        walletName='Ledger 1'
        isConnected={true}
        onCancel={onCancel}
        requestingConfirmation={true}
      />
    </StyledExtensionWrapper>
  )
}

_ConnectHardwareWallet.story = {
  name: 'Connect Hardware Wallet'
}
