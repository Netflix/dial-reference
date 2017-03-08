"use strict";

const colors  = require("colors/safe");
const sprintf = require("sprintf-js").sprintf;

function getParam(key) {
    var value;
    var args = process.argv.slice(2);
    args.forEach(function (param) {
        if(param.indexOf(key + "=") === 0) {
            value = param.split("=")[1];
        }
    });
    return value;
}

function printTestInfo(test, msg) {
    return console.log(sprintf("%-20s : %-s\n%-20s : %-s", "Test", test, "Description", msg));
}

function printSuccess() {
    return console.log(colors.green("TEST PASSED\n"));
}

function printFailure(err) {
    return console.log(colors.red(sprintf("%-20s : %-s\n", "TEST FAILED", err)));
}

module.exports.getParam       = getParam;
module.exports.printTestInfo  = printTestInfo;
module.exports.printSuccess   = printSuccess;
module.exports.printFailure   = printFailure;
