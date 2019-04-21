import * as React from 'react';
import { FileUpload } from '../file-upload/FileUpload';
import './Main.css';

interface State {
  // lookalikeFile: Blob | null;
  lookalikeFile: any;
}

export class Main extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      lookalikeFile: null,
    };
    this.showLookalike = this.showLookalike.bind(this);
  }

  render() {
    return (
      <div className="main">
        <h2>What celebrity is your lookalike?</h2>
        <div className="pane left">
          <FileUpload onResponseReceived={this.showLookalike}
                      onError={this.showError}/>
        </div>
        >
        <div className="pane right">
          <span>{this.state.lookalikeFile}</span>
        </div>
      </div>
    );
  }

  private showLookalike(lookalikeFile: Blob) {
    this.setState({ lookalikeFile: lookalikeFile.toString() });
  }

  private showError(data: any) {
    this.setState({ lookalikeFile: `error: ${data}` });
  }
}