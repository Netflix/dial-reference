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
    .help("help").alias("help", "h").argv;

function test() {
    var host = argv.host;
    return dial.sleepSystem(host)
        .then(function (response) {
            utils.printTestInfo(__filename.slice(__dirname.length + 1), "Sleep the system using DIAL server and key is required but is not provided and check for response code 403.");
            return response
        })
        .then(function (response) {

            if (response.statusCode !== 403) {
                throw new Error(`Error sleeping system without providing required key.  Expected to receive a 403 from the DIAL server but instead received ${response.statusCode}`);
            }
        })
        .then(function () {
            utils.printTestSuccess()
        })
        .catch(function handleError(err) {
            utils.printTestFailure(err);
        });
}

module.exports.test = test;

if (require.main === module) {
    test();
}
