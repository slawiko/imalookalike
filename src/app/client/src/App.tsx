import * as React from 'react';

import './App.css';

import { Header } from './components/header/Header';
import { Main } from './components/main/Main';

class App extends React.Component {
  public render() {
    return (
      <div>
        <Header/>
        <Main/>
      </div>
    );
  }
}

export default App;
