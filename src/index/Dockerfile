FROM alpine:3.9

ENV \
  INDEXUSER=indexuser \
  INDEXGROUP=indexgroup \
  DIR=/index \
  HTTPLIB_PATH=""

RUN \
  apk add --no-cache --update g++ && \
  rm -rf /var/cache/apk/*
  # TODO: clone https://github.com/yhirose/cpp-httplib and save path to HTTPLIB_PATH

RUN \
  addgroup -S ${INDEXGROUP} && \
  adduser -S ${INDEXUSER} -G ${INDEXGROUP} && \
  mkdir $DIR && \
  chown ${INDEXUSER}:${INDEXGROUP} $DIR
USER ${INDEXUSER}

WORKDIR $DIR

COPY ./ ./

RUN g++ --std=c++11 -pthread -O2 -x c++ -I$HTTPLIB_PATH main.cpp index.cpp thread_pool.cpp


