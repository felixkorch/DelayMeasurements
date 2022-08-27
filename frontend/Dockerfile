# ---------------------------------------
# Development stage
# ---------------------------------------

FROM alpine AS dev

RUN apk add nodejs yarn

WORKDIR /delay-frontend
COPY package.json ./
RUN yarn install

CMD yarn start

# ---------------------------------------
# Production stage
# ---------------------------------------

FROM alpine AS prod

RUN apk add nodejs yarn

WORKDIR /delay-frontend
COPY . ./
RUN yarn install

CMD yarn build