FROM node:10.15-alpine as client

ENV \
  APPUSER=appuser \
  APPGROUP=appgroup \
  CLIENT_DIR=/app

RUN \
  addgroup -S ${APPGROUP} && \
  adduser -S ${APPUSER} -G ${APPGROUP} && \
  mkdir $CLIENT_DIR && \
  chown ${APPUSER}:${APPGROUP} $CLIENT_DIR
USER ${APPUSER}

WORKDIR $CLIENT_DIR

# docker layers caching usage
COPY client/package.json client/package-lock.json ./
RUN npm install

COPY ./client/ ./
RUN npm run build

# slim-stretch because opencv is incompatible with alpine (or rather manylinux is not supporting alpine)
# TODO: find less fat image
FROM python:3.6-slim-stretch as server

RUN \
  apt-get update && \
  apt-get install -y cmake g++ libglib2.0-0 libsm6 libxext6 libxrender-dev

ENV \
  APPUSER=appuser \
  APPGROUP=appgroup \
  SERVER_DIR=/app

RUN \
  addgroup --system ${APPGROUP} && \
  adduser --system --ingroup ${APPGROUP} ${APPUSER} && \
  mkdir $SERVER_DIR && \
  chown ${APPUSER}:${APPGROUP} $SERVER_DIR
USER ${APPUSER}

WORKDIR $SERVER_DIR

ENV FLASK_ENV=production

# docker layers caching usage
COPY server/requirements.txt ./
RUN pip install --user -r ./requirements.txt --no-cache-dir

COPY --chown=appuser:appgroup ./server ./server
COPY --from=client /app/build ./client/build

WORKDIR $SERVER_DIR/server

EXPOSE 8000
ENTRYPOINT ["/usr/local/bin/python", "./app.py"]
