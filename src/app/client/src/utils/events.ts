import * as React from "react";

export function stopEvent(event: React.SyntheticEvent) {
  event.stopPropagation();
  event.preventDefault();
}
