import * as React from 'react'
import * as S from './style'
import PanelBox from '../../components/panel-box'
import { CaratStrongLeftIcon } from 'brave-ui/components/icons'
import locale from '../../constants/locale'

interface Props {
  onBack: Function
}

function SettingsPanel (props: Props) {
  const handleBackClick = () => props.onBack()

  return (
    <PanelBox>
      <S.PanelContent>
        <S.PanelHeader>
          <S.BackButton
            type='button'
            onClick={handleBackClick}
          >
            <i><CaratStrongLeftIcon /></i>
            <span>{locale.backLabel}</span>
          </S.BackButton>
        </S.PanelHeader>
        <S.List>
          <li>
            <a href='#' target='_blank' rel='noopener noreferrer'>
              {locale.sendFeedback}
            </a>
          </li>
          <li>
            <a href='#' target='_blank' rel='noopener noreferrer'>
              About Brave Firewall + VPN
            </a>
          </li>
          <li>
            <a href='#' target='_blank' rel='noopener noreferrer'>
              {locale.managePlan}
            </a>
          </li>
        </S.List>
      </S.PanelContent>
    </PanelBox>
  )
}

export default SettingsPanel
