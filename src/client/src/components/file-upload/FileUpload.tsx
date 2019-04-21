import * as React from 'react';

import './FileUpload.css';
import { stopEvent } from '../../utils/events';
import { ImageViewer } from '../image-viewer/ImageViewer';

interface Props {
  // onResponseReceived: (file: Blob) => void;
  onResponseReceived: (file: any) => void;
  onError: (data: any) => void;
}

interface State {
  mode: 'view' | 'upload',
  fileUrl: string | null,
  dragZoneClass: 'drag-in' | 'drag-out',
}

export class FileUpload extends React.Component<Props, State> {
  constructor(props: Props) {
    super(props);
    this.state = {
      mode: 'upload',
      fileUrl: null,
      dragZoneClass: 'drag-out',
    };
    this.handleDrop = this.handleDrop.bind(this);
    this.handleChoose = this.handleChoose.bind(this);
    this.dragIn = this.dragIn.bind(this);
    this.dragOut = this.dragOut.bind(this);
  }

  render() {
    return (
      <div onDrop={this.handleDrop}
           onDragOver={this.dragIn}
           onDragEnter={this.dragIn}
           onDragLeave={this.dragOut}>
        {(this.state.fileUrl && this.isViewMode())
          ? <ImageViewer fileUrl={this.state.fileUrl} alt="you"/>
          : <form className={`file-chooser ${this.state.dragZoneClass}`}>
            <input id="file-chooser" type="file" onChange={this.handleChoose}/>
            <label htmlFor="file-chooser">
              <strong>Choose your photo</strong>
              <span> or drag it here.</span>
            </label>
          </form>
        }
      </div>
    );
  }

  private isViewMode(): boolean {
    return this.state.mode === 'view';
  }

  private isUploadMode(): boolean {
    return this.state.mode === 'upload';
  }

  private dragIn(event: React.SyntheticEvent) {
    this.setState({
      dragZoneClass: 'drag-in',
      mode: 'upload',
    });
    stopEvent(event);
  }

  private dragOut(event: React.SyntheticEvent) {
    this.setState({
      dragZoneClass: 'drag-out',
      mode: 'view',
    });
    stopEvent(event);
  }

  private handleDrop(event: React.DragEvent) {
    this.handleFile(event.dataTransfer.files[0]);
    this.dragOut(event);
    stopEvent(event);
  }

  private handleChoose(event: React.ChangeEvent<HTMLInputElement>) {
    if (!event.target.files) {
      return;
    }
    this.handleFile(event.target.files[0]);
  }

  private handleFile(file: Blob) {
    this.showFile(file);
    this.uploadFile(file)
  }

  private showFile(file: Blob) {
    const reader = new FileReader();

    reader.readAsDataURL(file);

    reader.addEventListener('loadend', e => {
      this.setState({
        fileUrl: reader.result as string,
      });
    });
  }

  private uploadFile(file: Blob) {
    const reader = new FileReader();

    reader.readAsArrayBuffer(file);

    reader.addEventListener('loadend', e => {
      fetch('/upload', {
        method: 'POST',
        body: reader.result,
        headers: {
          'Content-type': 'image/jpeg'
        }
      })
        .then(res => {
          this.props.onResponseReceived(res);
        })
        .catch(e => {
          this.props.onError(e);
        });
    });
  }
}