"use strict";

const colors    = require("colors/safe");
const sprintf   = require("sprintf-js").sprintf;
const winston   = require("winston");
const moment    = require("moment");
const keypress  = require("keypress");
const Q         = require("q");

const levels = { error: 0, warn: 1, info: 2, verbose: 3, debug: 4 };
const transports = [
    new (winston.transports.Console)({
        level: "info",
        formatter: consoleFormatter
    }),
    new (winston.transports.File)({
        level: "debug",
        name: "log_file",
        filename: "js_tests_log.txt",
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
    case "debug": return colors.cyan(str);
    case "info":
    default: return str;
    }
}

function fileFormatter(options) {
    let str = options.message;

    return str;
}

function setLogLevel(level) {
    winston.remove(winston.transports.Console);
    return winston.add(winston.transports.Console, {
        level: level,
        formatter: consoleFormatter
    });
}

function ask(description) {
    return new Q.Promise(function (resolve, reject) {
        // make `process.stdin` begin emitting "keypress" events
        keypress(process.stdin);
        process.stdin.setRawMode(true);
        process.stdin.resume();

        winston.info(sprintf("[%-s] %-20s : %-s", moment().format("YYYY-MM-DDTHH:mm:ssZ"), "MANUAL STEP", description));

        // listen for the "keypress" event
        process.stdin.on("keypress", function (ch, key) {
            if (key && key.name === "return") {
                process.stdin.pause();
                return resolve();
            }
            if (key && key.name === "backspace") {
                process.stdin.pause();
                return reject("User marked the step as FAILED");
            }
        });
    });
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

function printInfo(msg) {
    winston.info(sprintf("[%-s] %-20s : %-s", moment().format("YYYY-MM-DDTHH:mm:ssZ"), "INFO", msg));
}

module.exports.printTestInfo      = printTestInfo;
module.exports.printTestSuccess   = printTestSuccess;
module.exports.printTestFailure   = printTestFailure;
module.exports.printDebug         = printDebug;
module.exports.printInfo          = printInfo;
module.exports.ask                = ask;
module.exports.setLogLevel        = setLogLevel;
