# ---------------------------------------
# Development stage
# ---------------------------------------

FROM python:3.8-slim-buster AS dev

WORKDIR /delay-backend
COPY requirements.txt .flaskenv ./

RUN python3 -m venv env && . env/bin/activate
RUN pip3 install -r requirements.txt

CMD python3 -m flask run --host=0.0.0.0

# ---------------------------------------
# Production stage
# ---------------------------------------
# TODO: Use actual production server

FROM python:3.8-slim-buster AS prod

WORKDIR /delay-backend
COPY . ./

RUN python3 -m venv env && . env/bin/activate
RUN pip3 install -r requirements.txt

CMD python3 -m flask run --host=0.0.0.0