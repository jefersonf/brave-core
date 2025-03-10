import * as React from 'react'
import { AccountAssetOptionType } from '../../../constants/types'
// Styled Components
import {
  StyledWrapper,
  AssetBalance,
  AssetAndBalance,
  AssetName,
  AssetIcon
} from './style'

export interface Props {
  asset: AccountAssetOptionType
  onSelectAsset: () => void
}

function SelectAssetItem (props: Props) {
  const { asset, onSelectAsset } = props

  return (
    <StyledWrapper onClick={onSelectAsset}>
      <AssetIcon icon={asset.asset.logo} />
      <AssetAndBalance>
        <AssetName>{asset.asset.name}</AssetName>
        <AssetBalance>{asset.asset.symbol}</AssetBalance>
      </AssetAndBalance>
    </StyledWrapper>
  )
}

export default SelectAssetItem
