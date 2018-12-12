
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
const MongoAuth = require('./auth.json');

const mongoose = require('mongoose');
const db = mongoose.connection;

db.on('error', console.error.bind(console, 'connection error:'));
db.once('open', function() {
  console.log("Successfully connected to the db!");
});

var Schema = mongoose.Schema;
var siteSchema = new Schema({
    name:  String,
    url: String,
    interval: Number,
    measurements: [{ date: Date, duration: Number, size: Number, code: Number }]
}, { versionKey: false });

siteSchema.statics.getAllNames = function getAllNames(cb) {
    this.find()
        .select('name')
        .lean()
        .exec(function(err, objs) {
            let res = objs.map((el) => {
                return el.name;
            });
            if(err || res == null) {
                cb(err, []);
                return;
            }
            cb(err, res);
        });
}

siteSchema.statics.getCalculations = function getCalculations(url, cb) {

    Site.aggregate([
        {
            $project: {
                _id: 0,
                name: 1,
                interval: 1,
                average: {
                    $avg: '$measurements.duration'
                },
                dataPoints: {
                    $size: '$measurements'
                },
                max: {
                    $max: '$measurements.duration'
                },
                min: {
                    $min: '$measurements.duration'
                },
                chartData: {
                    $slice: ['$measurements', -20]
                }
            }
        },
        {
            $match: {
                name: url
            }
        }
    ], function(err, result) {
        if(err || result == null) {
            cb(err, {});
            return;
        }
        result[0].success = true;
        cb(err, result[0]);
    });
    /*
    Site.findOne()
        .where('url').equals(url)
        .select('measurements interval')
        .lean()
        .exec(function(err, site) {
            let res = {};

            if(err || site == null) {
                cb(err, res);
                return;
            }
            
            res.measurements = site.measurements;

            res.durations = [];
            res.dates = [];
            res.measurements.forEach(x => {
                res.durations.push(x.duration);
                res.dates.push(new Date(x.date));
            });
            res.length = res.measurements.length;
            res.interval = site.interval;

            res.maxDuration = Math.max.apply(Math, res.durations);
            res.minDuration = Math.min.apply(Math, res.durations);
            res.average = res.durations.reduce((a, b) => a + b, 0) / res.durations.length;
            res.maxDeviation = Math.max(Math.abs(res.maxDuration - res.average), (Math.abs(res.minDuration - res.average)));

            let begin = 0;
            if(res.length >= 20)
                begin = res.length - 20;

            res.chartData = res.measurements.slice(begin, res.length).map( el => {
                return { date: new Date(el.date), value: el.duration };
            });

            res.success = true;
            cb(err, res);
        }); */
}

var Site = mongoose.model('Site', siteSchema, 'sites');

const httpServer = http.Server(app);
const socket = io.listen(httpServer);

var clientCount = 0;

function fileToJson(path, filename) {

    if (path.substring(path.length - 1) != "/")
        path += "/";

    let readSites;

    try {
        readSites = fs.readFileSync(path + filename, "utf8");
    }catch(err) {
        console.log("'" + filename + "' doesn't exist");
        return { success: false, json: {} };
    }
    return { success: true, json: JSON.parse(readSites) };
}


function addSite(url, cb) {
    if(!validator.isURL(url)) {
        cb({ success: false });
    }

    let skeleton = {
        "name": url,
        "url": url,
        "interval": 35000,
        "measurements": []
    };

    let newSite = new Site(skeleton);
    newSite.save(function (err) {
        if(err) {
            console.log("Problem with adding site: " + url);
            cb({ success: false });
        }else {
            console.log("Successfully added site: " + url);
            cb({ success: true });
        }
    });
}

function deleteSite(url, cb) {
    Site.deleteOne()
        .where('url').equals(url)
        .exec(function(err) {
            if (err){
                console.log("Problem with deleting site: " + url);
                cb({ success: false });
            }else {
                console.log("Successfully deleted site: " + url);
                cb({ success: true });
            }
        });
}

var sampleSizes = {};

socket.on('connection', function (socket) {

    console.log('user connected, id: ' + socket.id);
    clientCount++;
    socket.on('disconnect', function() {
        clientCount--;
        console.log('user disconnected');
    });

    socket.on('siteAdded', function (req, fn) {
        addSite(req.site, function(res) {
            fn(res);
        });
    });

    socket.on('siteDeleted', function (req, fn) {
        deleteSite(req.site, function(res) {
            fn(res);
        });
    });

    socket.on('siteList', function (fn) {
      Site.getAllNames(function(err, res) {
        if(err)
          fn({ success: false, list: [] });
        else
          fn({ success: true, list: res });
      });
    });

    socket.on('siteData', function (data, fn) {
        Site.getCalculations(data.siteName, function(err, res) {
            if(res.success && data.options.polling) {
                let size = sampleSizes[data.siteName];
                if(size == null) {
                    size = 0;
                    sampleSizes[data.siteName] = 0;
                    res.success = false;
                }
                if (size == res.dataPoints)
                    res.success = false;
                else
                    sampleSizes[data.siteName] = res.dataPoints;
            }
            fn(res);
        });
    });
});

app.get('/api/sites/:site', function(req, res, next) {
    Site.findOne()
        .where('name').equals(req.params.site)
        .select('measurements')
        .lean()
        .exec(function(err, site) {
            if(err) return next(err);
            if(site) {
                res.send(site.measurements.map((el) => {
                    return { date: new Date(el.date), value: el.duration };
                }));
            }else {
                res.send("Nothing here..")
            }
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

    mongoose.connect('mongodb://' + MongoAuth.user + ':' + MongoAuth.pass + '@' + MongoAuth.db, { autoIndex: false });

    // Listen the server
    httpServer.listen(port)
    consola.ready({
    message: `Server listening on http://${host}:${port}`,
    badge: true
    })
}
start()
