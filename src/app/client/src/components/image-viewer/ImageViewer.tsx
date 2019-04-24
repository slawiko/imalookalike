import * as React from 'react';

import './ImageViewer.css';

interface Props {
  fileUrl: string,
  alt?: string,
}

export class ImageViewer extends React.Component<Props, {}> {
  constructor(props: Props) {
    super(props);
  }

  render() {
    return (
      <img src={this.props.fileUrl} alt={this.props.alt} />
    );
  }
}