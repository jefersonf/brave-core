/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import {
  StyledWrapper,
  StyledTitle,
  StyledLoader,
  StyledError,
  StyledLink,
  StyledButton
} from './style'
import Modal from 'brave-ui/components/popupModals/modal/index'
import { LoaderIcon } from 'brave-ui/components/icons'
import { Button } from 'brave-ui/components'
import { getLocale, splitStringForTag } from '../../../../../common/locale'

export interface Props {
  id?: string
  errorText?: string
  buttonText?: string
  titleText?: string
  learnMore?: string
  walletType?: string
  displayCloseButton?: boolean
  onClick?: () => void
  onClose?: () => void
}

export default class ModalRedirect extends React.PureComponent<Props, {}> {

  getButton = () => {
    const { onClick, buttonText } = this.props
    if (!onClick || !buttonText) {
      return null
    }

    return (
      <StyledButton>
        <Button onClick={onClick} text={buttonText} type={'accent'} />
      </StyledButton>
    )
  }

  render () {
    const { id, errorText, titleText, learnMore, walletType, displayCloseButton, onClose } = this.props
    let tags = null
    if (errorText && errorText.includes('$1')) {
      tags = splitStringForTag(errorText)
    }

    let supportURL = ''
    if (walletType === 'uphold') {
      supportURL = 'https://uphold.com/en/brave/support'
    }

    return (
      <Modal id={id} displayCloseButton={!!displayCloseButton} onClose={onClose}>
        <StyledWrapper>
          <StyledTitle>
            {titleText}
          </StyledTitle>
          {
            errorText
            ? <StyledError>
              <p>
              {
                tags
                ? <>
                  {
                    supportURL
                    ? <a href={supportURL} target='_blank' rel='noopener noreferrer'>
                        {tags.duringTag}
                      </a>
                    : tags.duringTag
                  }
                  </>
                : errorText
              }
              </p>
              {
                learnMore &&
                  <StyledLink href={learnMore} target='_blank' rel='noopener noreferrer'>
                    {getLocale('learnMore')}
                  </StyledLink>
              }
              {this.getButton()}
            </StyledError>
            : <StyledLoader>
              <LoaderIcon />
            </StyledLoader>
          }
        </StyledWrapper>
      </Modal>
    )
  }
}
