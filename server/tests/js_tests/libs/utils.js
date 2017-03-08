"use strict";

const colors  = require("colors/safe");
const sprintf = require("sprintf-js").sprintf;

function printTestInfo(test, msg) {
    return console.log(sprintf("%-20s : %-s\n%-20s : %-s", "Test", test, "Description", msg));
}

function printTestSuccess() {
    return console.log(colors.green("TEST PASSED\n"));
}

function printTestFailure(err) {
    return console.log(colors.red(sprintf("%-20s : %-s\n", "TEST FAILED", err)));
}

module.exports.printTestInfo      = printTestInfo;
module.exports.printTestSuccess   = printTestSuccess;
module.exports.printTestFailure   = printTestFailure;
