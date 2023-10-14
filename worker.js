const addon = require('./build/Release/addon');
const array = new Uint8Array([1, 7, 4, 2, 3, 5, 6, 0]);
console.log(addon.add(array));