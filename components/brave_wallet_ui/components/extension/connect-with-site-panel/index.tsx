import * as React from 'react'

// Components
import {
  DividerLine,
  SelectAddress,
  ConnectBottomNav,
  ConnectHeader
} from '../'

// Styled Components
import {
  StyledWrapper,
  SelectAddressContainer,
  NewAccountTitle,
  SelectAddressScrollContainer,
  Details,
  MiddleWrapper,
  AccountListWrapper,
  ConfirmTextRow,
  ConfirmTextColumn,
  ConfirmText,
  ConfirmIcon
} from './style'

// Utils
import { reduceAddress } from '../../../utils/reduce-address'
import { WalletAccountType } from '../../../constants/types'
import { getLocale } from '../../../../common/locale'

export interface Props {
  siteURL: string
  accounts: WalletAccountType[]
  primaryAction: () => void
  secondaryAction: () => void
  selectAccount: (account: WalletAccountType) => void
  removeAccount: (account: WalletAccountType) => void
  selectedAccounts: WalletAccountType[]
  isReady: boolean
}
function ConnectWithSite (props: Props) {
  const {
    primaryAction,
    secondaryAction,
    removeAccount,
    selectAccount,
    siteURL,
    accounts,
    isReady,
    selectedAccounts
  } = props
  const checkIsSelected = (account: WalletAccountType) => {
    return selectedAccounts.some((a) => a.id === account.id)
  }

  const createAccountList = () => {
    const list = selectedAccounts.map((a) => {
      return reduceAddress(a.address)
    })
    return list.join(', ')
  }

  const toggleSelected = (account: WalletAccountType) => () => {
    if (checkIsSelected(account)) {
      removeAccount(account)
    } else {
      selectAccount(account)
    }
  }

  return (
    <StyledWrapper>
      <ConnectHeader url={siteURL} />
      <MiddleWrapper>
        {isReady ? (
          <AccountListWrapper>
            <Details>{createAccountList()}</Details>
          </AccountListWrapper>
        ) : (
          <Details>{getLocale('braveWalletConnectWithSiteTitle')}</Details>
        )}
        {!isReady ? (
          <SelectAddressContainer>
            <NewAccountTitle>{getLocale('braveWalletAccounts')}</NewAccountTitle>
            <DividerLine />
            <SelectAddressScrollContainer>
              {accounts.map((account) => (
                <SelectAddress
                  action={toggleSelected(account)}
                  key={account.id}
                  account={account}
                  isSelected={checkIsSelected(account)}
                />
              ))}
            </SelectAddressScrollContainer>
            <DividerLine />
          </SelectAddressContainer>
        ) : (
          <ConfirmTextRow>
            <ConfirmIcon />
            <ConfirmTextColumn>
              <ConfirmText>{getLocale('braveWalletConnectWithSiteDescription1')}</ConfirmText>
              <ConfirmText>{getLocale('braveWalletConnectWithSiteDescription2')}</ConfirmText>
            </ConfirmTextColumn>
          </ConfirmTextRow>
        )}
      </MiddleWrapper>
      <ConnectBottomNav
        primaryText={isReady ? getLocale('braveWalletAddAccountConnect') : getLocale('braveWalletConnectWithSiteNext')}
        secondaryText={isReady ? getLocale('braveWalletBack') : getLocale('braveWalletBackupButtonCancel')}
        primaryAction={primaryAction}
        secondaryAction={secondaryAction}
        disabled={selectedAccounts.length === 0}
      />
    </StyledWrapper>
  )
}

export default ConnectWithSite
