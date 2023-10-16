const { Worker, isMainThread } = require('worker_threads');
const SBLOCK_GENERATION_LENGTH = 10;

// activate workers thread
try {
    const workerFunctions = [];
    for (let i = 0; i < SBLOCK_GENERATION_LENGTH; ++i) {
        const byteArray = [];
        for (let num = 0; num < 16; ++num) {
            const byte = Math.floor((Math.random() * 256))%256;
            byteArray[num] = byte;
        }
        //console.log('byte array: ', byteArray);
        workerFunctions.push(checkSBlockAsync(byteArray));
    }
    const start = Date.now();
    Promise.all(workerFunctions).then((result)=> {
        console.info('calculating time: ', Date.now() - start);
    });
} catch (err) {
    console.error(err);
}

// burn up worker
async function checkSBlockAsync(bitArray) {
    if (isMainThread) {
        return new Promise((resolve, reject) => {
            const worker = new Worker('./worker.js', {
                workerData: bitArray
            });
            worker.on('message', resolve);
            worker.on('error', reject);
            worker.on('exit', (code) => {
                if (code !== 0) {
                    reject(new Error(`Worker stopped with exit code ${code}`));
                } else {
                    console.log('ending work of worker')
                }
            });
        });
    }
    return null;
}