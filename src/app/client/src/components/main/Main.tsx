import * as React from 'react';
import { FileUpload } from '../file-upload/FileUpload';
import './Main.css';

interface State {
  lookalikeFile: string | undefined;
}

export class Main extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      lookalikeFile: undefined,
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
          <img src={this.state.lookalikeFile}/>
        </div>
      </div>
    );
  }

  private showLookalike(lookalikeFile: Blob) {
    const reader = new FileReader();
    reader.readAsDataURL(lookalikeFile);
    reader.addEventListener('loadend', result => {
      this.setState({ lookalikeFile: reader.result as string });
    });
  }

  private showError(data: any) {
    // pass
  }
}