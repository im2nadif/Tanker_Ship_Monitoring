var fs = require('fs');
var caFile = fs.readFileSync("ca.crt");
var certFile = fs.readFileSync("esp.crt");
var keyFile = fs.readFileSync("esp.key");

const mqtt = require('mqtt');
var string = [],
    time, now, location = [],
    device, level, msg;

var opts = {
    rejectUnauthorized: false,
    username: "tigapagi",
    password: "pwdTigaPagi",
    connectTimeout: 5000,
    ca: [caFile],
    cert: certFile,
    key: keyFile,
}
const cloudPort = '15024';
const cloud = new mqtt.connect('mqtts://0.tcp.ap.ngrok.io:' + cloudPort, opts);

const {
    MongoClient
} = require('mongodb') const uri = 'mongodb://127.0.0.1:27017';
const dbName = 'm2ship_cloud';
const dbClient = new MongoClient(uri);

dbClient.connect((error, dbClient) => {
    if (error) {
        return console.log(error);
    }
    console.log('Local DB Connected');
    const db = dbClient.db(dbName);

    cloud.on('connect', function() {
        if (cloud.connected == true) {
            console.log("Cloud Server Connected")
        }
        cloud.subscribe('91025');
        cloud.subscribe('91026');
    });

    cloud.on('offline', function() {
        console.log("MQTT offline");
    });

    cloud.on('reconnect', function() {
        console.log("MQTT reconnecting");
    });

    cloud.on('message', function(topic, message) {
        string = message.toString().split(":");
        now = Date.now();
        time = new Date(string[2].replaceAll('/', '-').replace('#', ' ').replaceAll('|', ':')).getTime();
        location = string[3].split("@");
        device = string[0];
        level = string[1];
        console.log(device + ' ' + level + ' ' + time + ' ' + string[3]);
        db.collection('sensor').insertOne({
            device_id: parseInt(device),
            rtc: time,
            level: parseFloat(level),
            longitude: parseFloat(location[0]),
            latitude: parseFloat(location[1]),
            insert_time: now
        });
    });
});