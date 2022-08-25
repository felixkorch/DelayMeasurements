from bson import json_util, ObjectId
from flask import Flask, request
from flask_cors import CORS
import pymongo
import json
import os
from datetime import datetime

app = Flask(__name__)
CORS(app)

env_mongo_host = os.environ.get("ENV_HOST")
env_mongo_db = os.environ.get("ENV_DB")
env_user = os.environ.get("ENV_USER")
env_pass = os.environ.get("ENV_PASS")

mongo_port = 27017
mongo_host = env_mongo_host if env_mongo_host else "localhost"

mongo_uri = ""

if env_user is None:
    mongo_uri = "mongodb://" + mongo_host + ":" + str(mongo_port)
else:
    mongo_uri = "mongodb://" + env_user + ":" + env_pass + "@" + mongo_host + ":" + str(mongo_port)

mongo_db = env_mongo_db if env_mongo_db else "delay"
mongo_collection = "sites"

client = pymongo.MongoClient(mongo_uri)
db_handle = client[mongo_db]
sites = db_handle[mongo_collection]

class APIArgMissingError(Exception):
    code = 400
    description = "Bad Request, args missing/malformed"

class APIDBNotFound(Exception):
    code = 404
    description =  "Database failed to find requested resource"

class APIArgWrongType(Exception):
    code = 400
    description =  "Bad Request, arg has wrong type" 

def exception_handler(err):
    response = {
      "error": err.description
    }
    if len(err.args) > 0:
        response["message"] = err.args[0]
    return response, err.code

@app.errorhandler(APIArgMissingError)
def handle_exception(err):
    return exception_handler(err)

@app.errorhandler(APIDBNotFound)
def handle_exception(err):
    return exception_handler(err)

@app.errorhandler(APIArgWrongType)
def handle_exception(err):
    return exception_handler(err)

def parse_json(data):
    return json.loads(json_util.dumps(data))

# Return all measurements in the given interval
# args: name: String
#       date: { begin: Date, end: Date } format: %Y-%m-%d
@app.route("/api/measurements/date", methods=["GET"])
def api_get_measurements_by_date():
    if "name" not in request.args:
        raise APIArgMissingError
    if "date" not in request.args:
        raise APIArgMissingError
    date = json.loads(request.args["date"])
    begin = datetime.strptime(date["begin"], '%Y-%m-%d')
    end = datetime.strptime(date["end"], '%Y-%m-%d')
    pipeline = [
        {"$match": {"name": request.args["name"]}},
        {"$unwind": "$measurements"},
        {"$match": { "measurements.date": { "$gte": begin, "$lte": end } }},
        {"$project": { "date": "$measurements.date" , "duration": "$measurements.duration" , "code": "$measurements.code"  }}
    ]
    doc = sites.aggregate(pipeline);
    if doc is None:
         raise APIDBNotFound
    return parse_json(doc), 200

# Return all recent measurements
# args: name: String
#       limit: Number
@app.route("/api/measurements", methods=["GET"])
def api_get_measurements():
    if "name" not in request.args:
        raise APIArgMissingError
    if "limit" not in request.args:
        raise APIArgMissingError
    try:
        int(request.args["limit"])
    except:
        raise APIArgWrongType

    doc = sites.aggregate(pipeline=[
        {"$match": {"name": request.args["name"]}},
        {"$unwind": "$measurements"},
        {"$sort": { "measurements.date": -1 } },
        {"$limit": int(request.args["limit"]) },
        {"$project": { "date": "$measurements.date" ,
                       "duration": "$measurements.duration",
                       "code": "$measurements.code",
                       "size": "$measurements.size"  }}
    ]);
    if doc is None:
         raise APIDBNotFound
    return parse_json(doc), 200

# Add a site to monitor
@app.route("/api/sites", methods=["POST"])
def api_add_site():
    content = request.get_json()

    # Make sure fields exist
    if not "name" in content:
        raise APIArgMissingError
    if not "url" in content:
        raise APIArgMissingError
    if not "interval" in content:
        raise APIArgMissingError

    # Can interval be converted to an integer?
    interval = None
    try:
        interval = int(content["interval"])
    except:
        raise APIArgWrongType

    # Post!
    result = sites.insert_one({ "name": content["name"], "url": content["url"], "interval": interval })
    return { "status": "OK", "id": parse_json(result.inserted_id) }, 201

# Get all sites
@app.route("/api/sites", methods=["GET"])
def api_get_site():
    doc = sites.find({}, {"name": 1, "url": 1, "interval": 1})
    return parse_json(doc)

# Update site with id
@app.route("/api/sites/<oid>", methods=["PUT"])
def api_update_site(oid):
    content = request.get_json()

    # Make sure fields exist
    if not "name" in content:
        raise APIArgMissingError
    if not "url" in content:
        raise APIArgMissingError
    if not "interval" in content:
        raise APIArgMissingError

    # Can interval be converted to an integer?
    interval = None
    try:
        interval = int(content["interval"])
    except:
        raise APIArgWrongType

    # Update one with specific ID
    filter = {"_id": ObjectId(oid)}

    # Values to be updated.
    newData = { "$set": { "name": content["name"], "url": content["url"], "interval": interval } }

    # Update!
    sites.update_one(filter, newData);
    return { "status": "OK" }, 201

# Delete site with id
@app.route("/api/sites/<oid>", methods=["DELETE"])
def api_del_site(oid):
    # Delete one with specific ID
    filter = {"_id": ObjectId(oid)}

    # Delete!
    sites.delete_one(filter);
    return { "status": "OK" }, 200

# Get site with id
@app.route("/api/sites/<oid>", methods=["GET"])
def api_get_site_id(oid):
    doc = sites.find_one({"_id": ObjectId(oid)})
    return parse_json(doc)