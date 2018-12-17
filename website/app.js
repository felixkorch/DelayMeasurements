const createError = require('http-errors');
const express = require('express');
const path = require('path');
const cookieParser = require('cookie-parser');
const logger = require('morgan');
const validator = require('validator');
const fs = require("fs");
const bodyParser = require("body-parser");
const io = require('socket.io');
const MongoAuth = require('./auth.json');
const mongoose = require('mongoose');
const db = mongoose.connection;

const indexRouter = require('./routes/index');
const usersRouter = require('./routes/users');

function normalizePort(val) {
    var port = parseInt(val, 10);

    if (isNaN(port)) {
        // named pipe
        return val;
    }

    if (port >= 0) {
        // port number
        return port;
    }

    return false;
}
const app = express();
app.set('port', port);
const port = normalizePort(process.env.PORT || '3000');
const http = require("http");
const httpServer = http.Server(app);
const socket = io.listen(httpServer);

db.on('error', console.error.bind(console, 'connection error:'));
db.once('open', function () {
    console.log("Successfully connected to the db!");
});

var Schema = mongoose.Schema;
var siteSchema = new Schema({
    name: String,
    url: String,
    interval: Number,
    measurements: [{ date: Date, duration: Number, size: Number, code: Number }]
}, { versionKey: false });

siteSchema.statics.getAllNames = function getAllNames(cb) {
    this.find()
        .select('name')
        .lean()
        .exec(function (err, objs) {
            let res = objs.map((el) => {
                return el.name;
            });
            if (err || res == null) {
                cb(err, []);
                return;
            }
            cb(err, res);
        });
}

siteSchema.statics.getUIData = function getUIData(cb) {

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
        }
    ], function (err, result) {
        if (err || result == null) {
            cb(err, {});
            return;
        }
        result.success = true;
        cb(err, result);
    });
}

var Site = mongoose.model('Site', siteSchema, 'sites');

var clientCount = 0;
var siteData = {};

function fileToJson(path, filename) {

    if (path.substring(path.length - 1) != "/")
        path += "/";

    let readSites;

    try {
        readSites = fs.readFileSync(path + filename, "utf8");
    } catch (err) {
        console.log("'" + filename + "' doesn't exist");
        return { success: false, json: {} };
    }
    return { success: true, json: JSON.parse(readSites) };
}


function addSite(url, cb) {
    if (!validator.isURL(url)) {
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
        if (err) {
            console.log("Problem with adding site: " + url);
            cb({ success: false });
        } else {
            console.log("Successfully added site: " + url);
            cb({ success: true });
        }
    });
}

function deleteSite(url, cb) {
    Site.deleteOne()
        .where('url').equals(url)
        .exec(function (err) {
            if (err) {
                console.log("Problem with deleting site: " + url);
                cb({ success: false });
            } else {
                console.log("Successfully deleted site: " + url);
                cb({ success: true });
            }
        });
}

function getUIData() {

    Site.getUIData(function (err, res) {
        res.forEach(e => {
            if(!siteData[e.name]) {
                siteData[e.name] = e;
                return;
            }
            let changed = (e.dataPoints == siteData[e.name].dataPoints) ? 0 : 1;
            if(changed)
                socket.emit('site-changed', e);
            siteData[e.name] = e;
        });
    });

    setTimeout(getUIData, 1000);
}

getUIData();

socket.on('connection', function (socket) {
    clientCount++;
    console.log('user connected, id: ' + socket.id);

    socket.on('disconnect', function () {
        clientCount--;
        console.log('user disconnected');
    });

    socket.on('ui-data', function (req, cb) {
        if(siteData[req]) cb(siteData[req]);
    });

    socket.on('site-list', function(cb) {
        Site.getAllNames(function(err, res) {
            if(!err)
                cb(res);
        });
    });

    socket.on('add-site', function (req, fn) {
        addSite(req, function(res) {
            fn(res);
        });
    });
 
    socket.on('delete-site', function (req, fn) {
        deleteSite(req.site, function(res) {
            fn(res);
        });
    });
});

app.get('/api/sites/:site', function (req, res, next) {
    Site.findOne()
        .where('name').equals(req.params.site)
        .select('measurements')
        .lean()
        .exec(function (err, site) {
            if (err) return next(err);
            if (site) {
                res.send(site.measurements.map((el) => {
                    return { date: new Date(el.date), value: el.duration };
                }));
            } else {
                res.send("Nothing here..")
            }
        });
});

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.engine('html', require('ejs').renderFile);
app.set('view engine', 'html');

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

app.use('/', indexRouter);
app.use('/users', usersRouter);

// catch 404 and forward to error handler
app.use(function (req, res, next) {
    next(createError(404));
});

// error handler
app.use(function (err, req, res, next) {
    // set locals, only providing error in development
    res.locals.message = err.message;
    res.locals.error = req.app.get('env') === 'development' ? err : {};

    // render the error page
    res.status(err.status || 500);
    res.render('error');
});

mongoose.connect('mongodb://' + MongoAuth.user + ':' + MongoAuth.pass + '@' + MongoAuth.db, { autoIndex: false });
httpServer.listen(port);
module.exports = httpServer;