
const express = require('express')
const consola = require('consola')
const { Nuxt, Builder } = require('nuxt')
const app = express()
const http = require("http");
const host = process.env.HOST || '127.0.0.1'
const port = process.env.PORT || 3000

const validator = require("validator");
const fs = require("fs");
const path = require("path");
const bodyParser = require("body-parser");
const io = require('socket.io');
const MongoClient = require('mongodb').MongoClient;
const MongoHelper = require('./helper.js');
const MongoAuth = require('./auth.json');

const http_server = http.Server(app);
const socket = io.listen(http_server);

var db;
var mongo_helper = new MongoHelper(db, 'sites');
var client_count = 0;

class Site {

    static read_measurements(name, db_helper, callback) {
        let res = {};
        let query = { "url": name };
        db_helper.get_field("measurements", query, function(err, arr) {
            if(err || arr == null || arr == []) { // Fix empty array case
                callback(err, { "success": false });
                return;
            }
            res.success = true;
            res.measurements = arr;
            res.durations = arr.map(x => x["duration"]);
            res.times =  res.measurements.map(x => new Date(x["date"]));
            res.length = res.measurements.length;

            res.max_duration = Math.max.apply(Math, res.durations);
            res.min_duration = Math.min.apply(Math, res.durations);
            res.average = res.durations.reduce((a, b) => a + b, 0) / res.durations.length;
            res.max_deviation = Math.max(Math.abs(res.max_duration - res.average), (Math.abs(res.min_duration - res.average)));

            res.table_data = res.times.map( (t, i) => {
                return { date: t, value: res.durations[i] }
            });

            callback(err, res);
        });
    }

    static read_sites(db_helper, callback) {
        db_helper.get_field_many('name', {}, function(err, res) {
            if(err || res == null) {
                callback(err, []);
                return;
            }

            callback(err, res);
        });
    }

}

function file_to_json(path, filename) {

    if (path.substring(path.length - 1) != "/")
        path += "/";

    let read_sites;

    try {
        read_sites = fs.readFileSync(path + filename, "utf8");
    }catch(err) {
        console.log("'" + filename + "' doesn't exist");
        return { success: false, json: {} };
    }
    return { success: true, json: JSON.parse(read_sites) };
}


function add_site(name, callback) {
    if(!validator.isURL(name)) {
        callback({ success: false });
    }

    let obj = {
        "name": name,
        "url": name,
        "interval": 35000,
        "measurements": []
    };

    mongo_helper.insert_document(obj, function(err, res) {
        if(err) {
            console.log("Problem with adding site: " + name);
            callback({ success: false });
        }else {
            console.log("Successfully added site: " + name);
            callback({ success: true });
        }
    });
}

function delete_site(name, callback) {
    let query = {
        "url": name
    };
    mongo_helper.remove_document(query, function(err, res) {
        if(err) {
            console.log("Problem with deleting site: " + name);
            callback({ success: false });
        }else {
            console.log("Successfully deleted site: " + name);
            callback({ success: true });
        }
    });
}

var sample_sizes = {};

socket.on('connection', function (socket) {

    console.log('user connected');
    client_count++;
    socket.on('disconnect', function() {
        client_count--;
        console.log('user disconnected');
    });

    socket.on('site_added', function (req, fn) {
        add_site(req.site, function(res) {
            fn(res);
        });
    });

    socket.on('site_deleted', function (req, fn) {
        delete_site(req.site, function(res) {
            fn(res);
        });
    });

    socket.on('site_list', function (fn) {
      Site.read_sites(mongo_helper, function(err, res) {
        if(err)
          fn({ success: false, list: [] });
        else
          fn({ success: true, list: res });
      });
    });

    socket.on('site_data', function (data, fn) {
        Site.read_measurements(data.site_name, mongo_helper, function(err, res) {
            if(res.success && data.poll) {
                let size = sample_sizes[data.site_name];
                if(size == null) {
                    size = 0;
                    sample_sizes[data.site_name] = 0;
                }
                if (size == res.length)
                    res.success = false;
                else
                    sample_sizes[data.site_name] = res.length;
            }
            fn(res);
        });
    });
});


app.set('port', port)

// Import and Set Nuxt.js options
let config = require('../nuxt.config.js')
config.dev = !(process.env.NODE_ENV === 'production')

async function start() {
  // Init Nuxt.js
  const nuxt = new Nuxt(config)

  // Build only in dev mode
  if (config.dev) {
    const builder = new Builder(nuxt)
    await builder.build()
  }

  // Give nuxt middleware to express
  app.use(nuxt.render)

  MongoClient.connect('mongodb://' + MongoAuth.user + ':' + MongoAuth.pass + '@' + MongoAuth.db, (err, client) => {
    if (err) return console.log(err);
    db = client.db('delay');
    mongo_helper = new MongoHelper(db, 'sites');
});


  // Listen the server
  http_server.listen(port)
  consola.ready({
    message: `Server listening on http://${host}:${port}`,
    badge: true
  })
}
start()
