import * as React from 'react';
import { FileUpload } from '../file-upload/FileUpload';
import './Main.css';
import { ImageViewer } from '../image-viewer/ImageViewer';
import { Spinner } from '../spinner/Spinner';

interface State {
  lookalikeUrl: string | undefined;
  isLoading: boolean;
  error: string | undefined;
}

export class Main extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      lookalikeUrl: undefined,
      isLoading: false,
      error: undefined,
    };
    this.showLookalike = this.showLookalike.bind(this);
    this.showError = this.showError.bind(this);
    this.showSpinner = this.showSpinner.bind(this);
  }

  render() {
    return (
      <div className="main">
        <h2>What celebrity is your lookalike?</h2>
        <div className="pane left">
          <FileUpload onRequestStart={this.showSpinner}
                      onResponseReceived={this.showLookalike}
                      onError={this.showError}/>
        </div>
        <div className="pane right">
          {this.state.error && <div className="error">{this.state.error}</div>}
          {this.state.isLoading && !this.state.error && !this.state.lookalikeUrl && <Spinner />}
          {this.state.lookalikeUrl && !this.state.error && <ImageViewer fileUrl={this.state.lookalikeUrl} />}
        </div>
      </div>
    );
  }

  private showLookalike(lookalikeFile: Blob) {
    const reader = new FileReader();
    reader.readAsDataURL(lookalikeFile);
    reader.addEventListener('loadend', result => {
      this.setState({
        lookalikeUrl: reader.result as string,
        isLoading: false,
      });
    });
  }

  private showError(data: string) {
    this.setState({
      error: data,
      isLoading: false,
    });
  }

  private showSpinner() {
    this.setState({
      isLoading: true
    });
  }
}
