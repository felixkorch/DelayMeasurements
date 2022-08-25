FROM alpine

RUN apk add --update --no-cache \
    build-base \
    cmake \
    git \
    curl-dev \
    bash

RUN apk add mongo-cxx-driver-dev --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing

CMD /bin/bash && /delay-tool/gen_config.sh $ENV_USER $ENV_PASS $ENV_HOST $ENV_DB