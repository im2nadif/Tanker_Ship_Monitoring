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
const localPort = '16584';
const local = new mqtt.connect('mqtts://0.tcp.ap.ngrok.io:' + localPort, opts);
const cloud = new mqtt.connect('mqtts://0.tcp.ap.ngrok.io:' + cloudPort, opts);

const {
    MongoClient
} = require('mongodb') const uri = 'mongodb://127.0.0.1:27017';
const dbName = 'm2ship_local';
const dbClient = new MongoClient(uri);
dbClient.connect((error, dbClient) => {
    if (error) {
        return console.log('Local DB Failed');
    }

    console.log('Local DB Connected');
    const db = dbClient.db(dbName);

    local.on('connect', function() {
        if (local.connected == true) {
            console.log("Local Server Connected")
        }
        local.subscribe('91025');
        local.subscribe('91026');
    });
    local.on('offline', function() {
        console.log("MQTT offline");
    });
    local.on('reconnect', function() {
        console.log("MQTT reconnecting");
    });

    local.on('message', function(topic, message) {
        string = message.toString().split(":");
        now = Date.now();
        time = new Date(string[2].replaceAll('/', '-').replace('#', ' ').replaceAll('|', ':')).getTime();
        location = string[3].split("@");
        device = string[0];
        level = string[1];
        console.log(device + ' ' + level + ' ' + time + ' ' + string[3]);
        cloud.publish(device, message);
        if (!cloud.connected) {
            db.collection('sensor').insertOne({
                device_id: parseInt(device),
                rtc: time,
                level: parseFloat(level),
                longitude: parseFloat(location[0]),
                latitude: parseFloat(location[1]),
                insert_time: now
            });
        }
    });
});