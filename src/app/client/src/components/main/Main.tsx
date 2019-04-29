import * as React from 'react';
import { FileUpload } from '../file-upload/FileUpload';
import './Main.css';
import { ImageViewer } from '../image-viewer/ImageViewer';

interface State {
  lookalikeUrl: string | undefined;
  error: string | undefined;
}

export class Main extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      lookalikeUrl: undefined,
      error: undefined,
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
        <div className="pane right">
          {this.state.error && <div className="error">{this.state.error}</div>}
          {this.state.lookalikeUrl && !this.state.error && <ImageViewer fileUrl={this.state.lookalikeUrl} />}
        </div>
      </div>
    );
  }

  private showLookalike(lookalikeFile: Blob) {
    const reader = new FileReader();
    reader.readAsDataURL(lookalikeFile);
    reader.addEventListener('loadend', result => {
      this.setState({ lookalikeUrl: reader.result as string });
    });
  }

  private showError(data: string) {
    this.setState({
      error: data
    });
  }
}