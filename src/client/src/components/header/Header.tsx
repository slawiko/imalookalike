import * as React from 'react';

import './Header.css';

export class Header extends React.Component {
  public render() {
    const { host } = window.location;
    const items: {[key: string]: string} = {};

    items[host] = '/';
    return (
      <header className="header">
        Header
      </header>
    );
  }
}