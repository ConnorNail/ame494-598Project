var MS = require("mongoskin");
var mongo = require('mongodb');
var MongoClient = require('mongodb').MongoClient;
var express = require("express");
var app = express();
var bodyParser = require('body-parser');
var errorHandler = require('errorhandler');
var methodOverride = require('method-override');
var hostname = process.env.HOSTNAME || 'localhost';
var port = 1234;
var qte = require('quaternion-to-euler');

var url = "mongodb://localhost:27017/robotSensorData";

// app.use(bodyParser.json());

app.get("/", function (req, res) {
  res.redirect("index.html")
});

var values = {
  '24:0A:C4:08:A6:D0': { id: '24:0A:C4:08:A6:D0', x: 0, y: 0, z: 0, w: 1 },
  '08:3A:F2:44:C8:E8': { id: '08:3A:F2:44:C8:E8', x: 0, y: 0, z: 0, w: 1 }
};
app.get("/setValue", function (req, res) {
  res.send("1");
  var euler = qte([parseFloat(req.query.w), parseFloat(req.query.x), parseFloat(req.query.y), parseFloat(req.query.z)]);
  req.query.w = parseFloat(req.query.w);
  req.query.x = parseFloat(req.query.x);
  req.query.y = parseFloat(req.query.y);
  req.query.z = parseFloat(req.query.z);
  req.query.xE = euler[0]
  req.query.yE = euler[1]
  req.query.zE = euler[2]
  values[req.query.id] = req.query
});

app.get("/getValue", function (req, res) {
  res.send(values[req.query.id]);
});

app.get("/setCalibration", function (req, res) {
  res.send("1");
  req.query.x = parseFloat(req.query.x);
  req.query.y = parseFloat(req.query.y);
  req.query.z = parseFloat(req.query.z);
  req.query.w = parseFloat(req.query.w);
  values[req.query.id] = req.query;
  console.log(values);
});


app.use(methodOverride());
app.use(bodyParser());
app.use(express.static(__dirname + '/public'));
app.use(errorHandler());

console.log("Simple static server listening at http://" + hostname + ":" + port);
app.listen(port);



const WebSocket = require('ws');
const e = require("express");

const wss = new WebSocket.Server({ port: 3000 })

MongoClient.connect(url, function (err, db) {
  if (err) throw err;
  var dbo = db.db("robotSensorData");


  wss.on('connection', ws => {
    ws.on('message', message => {
      //console.log(`Received message => ${message}`)

      var myobj = JSON.parse(message.toString());
      myobj.time = new Date().getTime();
      myobj.xRotCal = values[myobj.id].x;
      myobj.yRotCal = values[myobj.id].y;
      myobj.zRotCal = values[myobj.id].z;
      myobj.wRotCal = values[myobj.id].w;
      console.log(myobj)
      dbo.collection("robotSensorDemo").insertOne(myobj, function (err, res) {
        if (err) throw err;
        //console.log("1 document inserted");
        //db.close();
      });

      wss.clients.forEach(function each(client) {
        if (client !== ws && client.readyState === WebSocket.OPEN) {
          client.send(message, { binary: false });
        }
      });
    })
    //ws.send('start');
  })
});

