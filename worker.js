const { workerData, parentPort } = require('worker_threads');
const addon = require('./build/Release/addon');

var byteArray = workerData;
var data = addon.add(new Uint8Array(byteArray));
parentPort.postMessage(data);