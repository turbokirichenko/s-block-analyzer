const { Worker, isMainThread } = require('worker_threads');
const SBLOCK_GENERATION_LENGTH = 10;
const SBLOCK_LENGTH = 32;
const { appendFileSync, writeFileSync } = require('fs');
const filePath = __dirname + '/report.txt';

// activate workers thread
try {
    const workerFunctions = [];
    const byteArray = [];
    for (let i = 0; i < SBLOCK_GENERATION_LENGTH; ++i) {
        for (let num = 0; num < SBLOCK_LENGTH; ++num) {
            const byte = Math.floor((Math.random() * 256))%256;
            byteArray[num] = byte;
        }
        console.log('byte array: ', byteArray.map(byte => byte.toString(16)));
        workerFunctions.push(checkSBlockAsync(byteArray));
    }
    const start = Date.now();
    const waiting = Promise.all(workerFunctions).then((result)=> {
        console.info('calculating time: ', Date.now() - start);
        writeFileSync(filePath, 'check-blocks: ' + '\n');
        appendFileSync(filePath, 'bytes: ' + byteArray.map(byte => byte.toString(16)).join(', ') + '\n');
        return result;
    });
    waiting.then(resultArray => {
        const res = resultArray.map(printResult);
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
                    // console.log('ending work of worker')
                }
            });
        });
    }
    return null;
}

// printing result of the block
function printResult(data) {
    appendFileSync(filePath, 'check-s-block: \n');
    let resultMatrix = Array(SBLOCK_LENGTH).fill(0);
    data.map((matrix) => {
        matrix.map((v, i) => {
            resultMatrix[i] += v/SBLOCK_LENGTH;
        });
    });
    var localStack = [];
    resultMatrix.map((v, i) => {
        localStack.push(v - 0.5);
        if ((i + 1)%4 == 0) {
            appendFileSync(filePath, localStack.join(', ') + '\n');
            localStack = [];
        }
    });
    appendFileSync(filePath, '\n');
    var done = true;
    return done;
}