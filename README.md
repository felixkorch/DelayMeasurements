
## Build instructions
1. Create a .env file in the root directory of the project and add a few variables:

```
DB_USER=defaultUser
DB_PASS=defaultPass
DB_TO_USE=delay
DB_HOST=delay-db
STAGE=dev
```
2. To set-up a dev environment using docker-compose, run:
```
docker-compose up -d
```

Note: This will mirror your local folders into the container, so changes will automatically be applied.


## Building the web-server separately

### Backend: 
1. Create a new virtual environment and activate it
```
python -m venv env && source env/Scripts/activate
```
2. Export two variables in current terminal so the backend can connect to the DB.
```
export ENV_USER=defaultUser
export ENV_PASS=defaultPass
```
3.  Run this to install dependencies
```
pip install -r requirements.txt
```
4. Start the server:
```
flask run
```

### Frontend: 
1. Install yarn if you haven't
2. Run this from ./frontend to install dependencies:
```
yarn install
```
3. Start the server

```
yarn start
```
