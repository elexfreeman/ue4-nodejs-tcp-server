"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.fGenerateToken = exports.aSocketClient = void 0;
const net = __importStar(require("net"));
const crypto = __importStar(require("crypto"));
const moment = require('moment');
const uniqid = require('uniqid');
/* клиенты */
exports.aSocketClient = {};
/**
 * Генерирует токен
 */
function fGenerateToken() {
    return crypto
        .createHash('md5')
        .update(uniqid(), 'utf8')
        .digest('hex');
}
exports.fGenerateToken = fGenerateToken;
/**
 * Текущая дата
 */
const fGetNowDataStr = () => moment().format('DD.MM.YYYY HH:mm:ss');
/**
 * Оработчик сервера
 */
const server = net.createServer((socket) => {
    /* генерируем токен клиенту */
    const clientToken = fGenerateToken();
    exports.aSocketClient[clientToken] = true;
    console.log(`[${fGetNowDataStr()}] Client connect ${clientToken}`);
    /* отправка через интервал  */
    const t = setInterval(() => {
        if (socket) {
            socket.write('Ping test');
        }
    }, 3000);
    /* получение данных от клиента */
    socket.on('data', (data) => {
        console.log(`[${fGetNowDataStr()}] Data from [${clientToken}]: `, data.toString());
    });
    /* клиент отключися */
    socket.on('end', () => {
        clearInterval(t);
        delete exports.aSocketClient[clientToken];
        console.log(`[${fGetNowDataStr()}] Client ${clientToken} disconnect`);
    });
});
/* ошибки сервера */
server.on('error', (err) => {
    console.log(`[${fGetNowDataStr()}] Error:`, err);
});
/* запускаем сервер */
server.listen({
    port: 3007, family: 'IPv4', address: '127.0.0.1'
}, () => {
    console.log('opened server on', server.address());
});
//# sourceMappingURL=tcpSrv.js.map