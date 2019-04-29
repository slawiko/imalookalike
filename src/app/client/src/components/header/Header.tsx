import * as React from 'react';

import './Header.css';

export class Header extends React.Component {
  public render() {
    const { host } = window.location;
    const items: {[key: string]: string} = {};

    items[host] = '/';
    return (
      <header className="header">
        <span className="logo">imalookalike</span>
        <span className="about">
          <a href="https://github.com/slawiko/imalookalike/blob/master/README.md">About</a>
        </span>
      </header>
    );
  }
}
