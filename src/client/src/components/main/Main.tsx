import * as React from 'react';

import './Main.css';

export class Main extends React.Component<{}> {
  constructor(props: {}) {
    super(props);
    this.handleFileUpload = this.handleFileUpload.bind(this);
  }

  public render() {
    return (
      <div className="main">
        <input type="file" onChange={this.handleFileUpload} />
      </div>
    );
  }

  public handleFileUpload(event: React.ChangeEvent<HTMLInputElement>) {
    const formData = new FormData();
    if (!event.target.files) {
      return;
    }

    const reader = new FileReader();

    reader.readAsArrayBuffer(event.target.files[0]);

    reader.addEventListener('loadend', e => {
      fetch('/upload', {
        method: 'POST',
        body: reader.result,
        headers: {
          'Content-type': 'image/jpeg'
        }
      })
      .then(res => {
        console.log(res);
      });
    });
  }
}