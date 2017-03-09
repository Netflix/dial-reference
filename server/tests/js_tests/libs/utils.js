"use strict";

const colors  = require("colors/safe");
const sprintf = require("sprintf-js").sprintf;
const winston = require("winston");
const moment  = require("moment");

const levels = { error: 0, warn: 1, info: 2, verbose: 3, debug: 4 };
const transports = [
    new (winston.transports.Console)({
        level: "info",
        formatter: consoleFormatter
    }),
    new (winston.transports.File)({
        level: "debug",
        name: "log_file",
        filename: "log.txt",
        json: false,
        formatter: fileFormatter,
        options: { flags: "w" }
    }),
];

winston.configure({ levels: levels, transports: transports });

function consoleFormatter(options) {
    let str = options.message;

    switch (options.level) {
    case "error": return colors.red(str);
    case "warn": return colors.yellow(str);
    case "debug": return colors.blue(str);
    case "info":
    default: return str;
    }
}

function fileFormatter(options) {
    let str = options.message;

    return str;
}

function printTestInfo(testFile, description) {
    winston.info(sprintf("[%-s] %-20s : %-s", moment().format("YYYY-MM-DDTHH:mm:ssZ"), "TEST", testFile));
    winston.info(sprintf("[%-s] %-20s : %-s", moment().format("YYYY-MM-DDTHH:mm:ssZ"), "DESCRIPTION", description));
}

function printTestSuccess() {
    winston.info(sprintf("[%-s] %-20s : %-s", moment().format("YYYY-MM-DDTHH:mm:ssZ"), "RESULT", "TEST PASSED\n"));
}

function printTestFailure(err) {
    winston.error(sprintf("[%-s] %-20s : %-s", moment().format("YYYY-MM-DDTHH:mm:ssZ"), "RESULT", "TEST FAILED " + err + "\n"));
}

function printDebug(msg) {
    winston.debug(sprintf("[%-s] %-20s : %-s", moment().format("YYYY-MM-DDTHH:mm:ssZ"), "DEBUG", msg));
}

module.exports.printTestInfo      = printTestInfo;
module.exports.printTestSuccess   = printTestSuccess;
module.exports.printTestFailure   = printTestFailure;
module.exports.printDebug         = printDebug;
