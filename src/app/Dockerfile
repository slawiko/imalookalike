FROM python:3.6-alpine

ENV \
  APPUSER=appuser \
  APPGROUP=appgroup

RUN \
  addgroup -S ${APPGROUP} && \
  adduser -S ${APPUSER} -G ${APPGROUP} && \
  mkdir /app && \
  chown ${APPUSER}:${APPGROUP} /app
USER ${APPUSER}

WORKDIR /app

# docker layers caching usage
COPY ./requirements.txt ./
RUN pip install --user -r ./requirements.txt --no-cache-dir

COPY ./ ./

EXPOSE 8000
ENTRYPOINT [ "/usr/local/bin/python", "./app.py" ]
