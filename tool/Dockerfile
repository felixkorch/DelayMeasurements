# ---------------------------------------
# Development stage
# ---------------------------------------
FROM alpine AS dev

ARG ARG_USER
ARG ARG_PASS
ARG ARG_HOST
ARG ARG_DB
ARG ARG_STAGE=dev

ENV ENV_USER $ARG_USER
ENV ENV_PASS $ARG_PASS
ENV ENV_HOST $ARG_HOST
ENV ENV_DB $ARG_DB
ENV ENV_STAGE=${ARG_STAGE}

WORKDIR /delay-tool

RUN apk add --update --no-cache \
    build-base \
    cmake \
    git \
    curl-dev \
    bash

RUN apk add mongo-cxx-driver-dev --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing

CMD ["tail", "-f", "/dev/null"]

# ---------------------------------------
# Production stage
# ---------------------------------------

FROM alpine AS prod

ARG ARG_USER
ARG ARG_PASS
ARG ARG_HOST
ARG ARG_DB
ARG ARG_STAGE=prod

ENV ENV_USER $ARG_USER
ENV ENV_PASS $ARG_PASS
ENV ENV_HOST $ARG_HOST
ENV ENV_DB $ARG_DB
ENV ENV_STAGE=${ARG_STAGE}

WORKDIR /delay-tool

RUN apk add --update --no-cache \
    build-base \
    cmake \
    git \
    curl-dev \
    bash

COPY . ./

RUN chmod +x ./build_server.sh
RUN /bin/bash && /delay-tool/build_server.sh

ENTRYPOINT cd /delay-tool/reqserver/build/bin && ./reqserver