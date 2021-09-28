import styled from 'styled-components'

export const List = styled.ul`
  list-style-type: none;
  padding: 0;
  text-align: center;

  li {
    margin-bottom: 28px;

    &:last-child {
      margin-bottom: 0;
    }
  }

  a {
    font-family: ${(p) => p.theme.fontFamily.heading};
    font-size: 13px;
    font-weight: 600;
    color: ${(p) => p.theme.color.interactive05};
  }
`

export const PanelContent = styled.section`
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 0 0 25px 0;
  z-index: 2;
`

export const PanelHeader = styled.section`
  display: flex;
  align-items: center;
  width: 100%;
  padding: 18px 18px 0 18px;
  margin-bottom: 18px;
  box-sizing: border-box;
`

export const BackButton = styled.button`
  background-color: transparent;
  border: 0;
  padding: 0;
  display: flex;
  align-items: center;

  span {
    font-family: ${(p) => p.theme.fontFamily.heading};
    font-size: 13px;
    font-weight: 600;
    color: ${(p) => p.theme.color.interactive05};
  }

  i {
    width: 18px;
    height: 18px;
    margin-right: 5px;
  }

  svg > path {
    fill: ${(p) => p.theme.color.interactive05};
  }
`
