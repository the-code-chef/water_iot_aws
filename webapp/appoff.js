var awsIot = require('aws-iot-device-sdk');

/*
var device=awsIot.device({
    keyPath:'5a26619c73-private.pem.key',
    certPath:'5a26619c73-certificate.pem.crt',
    caPath:'rootCA.pem',
    clientId:'MyBus',
    region:'ap-southeast-1'
});
*/
// Deepanshu
var device = awsIot.device({
    keyPath:'3638cf44dc-private.pem.key' ,
    certPath: '3638cf44dc-certificate.pem.crt',
    caPath: 'Deepanshu/rootCA.pem',
    clientId:'Dec_Water',
    host:'afe4foq5ne6fo-ats.iot.ap-southeast-1.amazonaws.com'
});

// Sushant
/*
var device = awsIot.device({
    keyPath:'14e799da83-private.pem.key' ,
    certPath: '14e799da83-certificate.pem.crt',
    caPath: 'ArootCA.pem',
    clientId:'Dec_2019_Water',
    host:'afe4foq5ne6fo-ats.iot.ap-southeast-1.amazonaws.com'
});
*/

var contents ="Started.....!!!!";

device
    .on('connect',function () {

        console.log('connect');
        //device.subscribe(busPolicy);
        device.publish('$aws/things/Dec_Water/shadow/update',JSON.stringify({"state":{"reported":{"test_value1":200, "test_value2":200,"test_value3":200}}}));
        console.log('Message Sent...');

    });

device
    .on('message',function (topic,payload) {
        console.log('message',topic,payload.toString());

    });
