import * as net from "net";
import * as crypto from "crypto";

const moment = require('moment');
const uniqid = require('uniqid');

export interface SocketClientI {
    [key: string]: any;
}

/* клиенты */
export const aSocketClient: SocketClientI = {};

/**
 * Генерирует токен
 */
export function fGenerateToken(): string {
    return crypto
        .createHash('md5')
        .update(uniqid(), 'utf8')
        .digest('hex');
}

/**
 * Текущая дата
 */
const fGetNowDataStr = (): string => moment().format('DD.MM.YYYY HH:mm:ss');

/**
 * Оработчик сервера
 */
const server = net.createServer((socket: net.Socket) => {

    /* генерируем токен клиенту */
    const clientToken = fGenerateToken();
    aSocketClient[clientToken] = true;

    console.log(`[${fGetNowDataStr()}] Client connect ${clientToken}`);

    /* отправка через интервал  */
    const t = setInterval(() => {
        if (socket) {
            socket.write('Ping test');
        }
    }, 3000);


    /* получение данных от клиента */
    socket.on('data', (data: Buffer) => {
        console.log(`[${fGetNowDataStr()}] Data from [${clientToken}]: `, data.toString());
    });

    /* клиент отключися */
    socket.on('end', () => {
        clearInterval(t);
        delete aSocketClient[clientToken];
        console.log(`[${fGetNowDataStr()}] Client ${clientToken} disconnect`);
    });

});

/* ошибки сервера */
server.on('error', (err: any) => {
    console.log(`[${fGetNowDataStr()}] Error:`, err);
});


/* запускаем сервер */
server.listen({
    port: 3007, family: 'IPv4', address: '127.0.0.1'
}, () => {
    console.log('opened server on', server.address());
});