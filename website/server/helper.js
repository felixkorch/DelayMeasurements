const create_object = (key, val) => { let obj = {}; obj[key] = val; return obj; };

class MongoHelper {
    constructor(database, collection) {
        this.database = database;
        this.collection = collection;
    }

    remove_document(query, callback) {
        this.database.collection(this.collection).deleteOne(
            query, function(err, res) {
                callback(err, res);
            }
        );
    }

    insert_document(obj, callback) {
        this.database.collection(this.collection).insertOne(
            obj, function(err, res) {
                callback(err, res);
            }
        );
    }

    push_to_array(field, obj) {
        this.database.collection(this.collection).updateOne(
            {},
            {
                $push: create_object(field, obj)
            }
        );
    }

    get_field(field, query, callback) {
        let projection = {};
        projection[field] = 1; // How to only include one?
        projection["_id"] = 0; 
        this.database.collection(this.collection).findOne(
            query,
            projection,
            function(err, res) {
                if(res)
                    callback(err, res[field]);
                else
                    callback(err, null);
            }
        );
    }

    get_field_many(field, query, callback) {
        
        let projection = {};
        projection[field] = 1;
        projection["_id"] = 0;

        this.database.collection(this.collection).find(
            query,
            projection
        ).toArray(function(err, res) {
            if(res)
                callback(err, res.map(a => a[field]));
            else
                callback(err, null);
        });
    }
};

module.exports = MongoHelper;
