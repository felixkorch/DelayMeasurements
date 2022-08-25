FROM alpine

RUN apk add --update --no-cache \
    build-base \
    cmake \
    git \
    curl-dev \
    bash

RUN apk add mongo-cxx-driver-dev --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing

WORKDIR /delay-tool
COPY ./ /delay-tool

RUN cd /delay-tool/reqserver && mkdir -p build
RUN cd /delay-tool/reqserver/build && cmake .. && make
RUN chmod +x /delay-tool/gen_config.sh

CMD /delay-tool/gen_config.sh $ENV_USER $ENV_PASS $ENV_HOST $ENV_DB && \
    cd /delay-tool/reqserver/build/bin && mv -f /delay-tool/reqserver.json ./ && ./reqserver