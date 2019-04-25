FROM alpine:3.9 as build

ENV \
  INDEXUSER=indexuser \
  INDEXGROUP=indexgroup \
  DIR=/index \
  HTTPLIB_PATH=/httplib

RUN \
  apk add --no-cache --update g++ && \
  rm -rf /var/cache/apk/*

RUN \
  addgroup -S ${INDEXGROUP} && \
  adduser -S ${INDEXUSER} -G ${INDEXGROUP} && \
  mkdir ${DIR} && \
  mkdir ${HTTPLIB_PATH} && \
  chown ${INDEXUSER}:${INDEXGROUP} ${DIR} && \
  chown ${INDEXUSER}:${INDEXGROUP} ${HTTPLIB_PATH}
USER ${INDEXUSER}

ADD --chown=indexuser:indexgroup https://github.com/yhirose/cpp-httplib/archive/master.zip /tmp/
RUN unzip /tmp/master.zip -d ${HTTPLIB_PATH}

WORKDIR $DIR

COPY --chown=indexuser:indexgroup ./ ./

RUN \
  g++ --std=c++11 -o index -pthread -O2 -x c++ -I${HTTPLIB_PATH}/cpp-httplib-master main.cpp index.cpp thread_pool.cpp arguments.cpp && \
  ls -la

EXPOSE 8000

ENTRYPOINT ["./index"]

# TODO:
#FROM alpine:3.9
#
#ENV \
#  INDEXUSER=indexuser \
#  INDEXGROUP=indexgroup \
#  DIR=/index
#
#RUN \
#  addgroup -S ${INDEXGROUP} && \
#  adduser -S ${INDEXUSER} -G ${INDEXGROUP} && \
#  mkdir ${DIR} && \
#  chown ${INDEXUSER}:${INDEXGROUP} ${DIR}
#USER ${INDEXUSER}
#
#WORKDIR ${DIR}
#
#COPY --from=build /index/index .
#
#EXPOSE 8000
#
#CMD ["./index"]
