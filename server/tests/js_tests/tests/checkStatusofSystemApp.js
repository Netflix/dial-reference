"use strict";

var dial = require("../libs/dialClient.js"),
    utils = require("../libs/utils.js");

const argv = require("yargs")
    .usage("\nUsage: node " + __filename.slice(__dirname.length + 1) + "[options]")
    .option("host", {
        describe: "IP address of host on which DIAL server under test is running",
        type: "string",
        demand: true
    })
    .option("application", {
        alias: "app",
        describe: "Application to test",
        type: "string",
        demand: true
    })
    .option("timeToWaitForStateChange", {
        alias: "ttw",
        describe: "Time(ms) to wait between state changes before querying application status",
        type: "string",
        default: 5000
    })
    .option("logLevel", {
        alias: "loglevel",
        describe: "The log level for displaying messages to the console, default is `info`', options are info, debug, error, warn",
        type: "string",
        default: "info"
    })
    .help("help").alias("help", "h").argv;

function test() {
    utils.setLogLevel(argv.logLevel);
    var host = argv.host;
    var systemApp = "system";
    return Promise.resolve()
        .then(() => {
            utils.printTestInfo(__filename.slice(__dirname.length + 1), `Confirm ${systemApp} is in the hidden state.`);
        })
        .then(() => {
            utils.printDebug("Querying application state");
        })
        .then(dial.getApplicationStatus.bind(null, host, systemApp))
        .then((result) => {
            if (!result || !result.state) {
                return Q.reject(new Error("Error retrieving current " + app + " application state"));
            }
            return result.state;
        })
        .then((state) => {

            if (state !== "hidden") {
                throw new Error(`The system app should only be in the hidden state but it was in ${state} state.`);
            }
            else {
                utils.printDebug(`Confirmed application is in ${state} state`);
            }
        })
        .then(() => {
            utils.printDebug("Attempting to stop system application.");
        })
        .then(dial.stopApplication.bind(null, host, systemApp))
        .then((response) => {
            if(response.statusCode !== 403) {
                throw new Error(`Tried to stop ${systemApp}. Expected statusCode: 403 but got ${response.statusCode}`);
            }
            utils.printDebug(`Confirmed response to stop request is 403 as expected.`);
        })
        .then(dial.launchApplication.bind(null, host, systemApp))
        .then((response) => {
            if(response.statusCode !== 403) {
                throw new Error(`Tried to launch ${systemApp}. Expected statusCode: 403 but got ${response.statusCode}`);
            }
            utils.printDebug(`Confirmed response to launch request is 403 as expected.`);
        })
        .then(() => {
            utils.printTestSuccess()
        })
        .catch((err) => {
            utils.printTestFailure(err);
        });
}
module.exports.test = test;

if (require.main === module) {
    test();
}
