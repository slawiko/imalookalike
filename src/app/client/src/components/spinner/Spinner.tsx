import * as React from "react";

export class Spinner extends React.Component<{}, {}> {
  constructor(props: {}) {
    super(props);
  }

  render() {
    return (
      <div className="spinner-container">
        <span>Imagine it spins</span>
      </div>
    );
  }
}