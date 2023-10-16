const { workerData, parentPort } = require('worker_threads');
const addon = require('./build/Release/addon');

const byteArray = workerData;
parentPort.postMessage(addon.add(new Uint8Array(byteArray)));