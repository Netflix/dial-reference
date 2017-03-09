"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

const argv    = require("yargs")
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
        describe: "Time(ms) to wait between state changes before Querying application state",
        type: "string",
        default: 5000
    })
    .help("help").alias("help", "h").argv;

function test() {
    var host = argv.host;
    var app = argv.application;
    var timeToWaitForStateChange = argv.timeToWaitForStateChange || 5000;

    return new Q()
      .then(function () {
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Hide " + app + " application when it is running and expect response code 200");
      })
      .then(function () {
          utils.printDebug("Querying application state ..");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          return result.state;
      })

      .then(function startAppIfNotRunning(state) {
          utils.printDebug("Application is in " + state + " state");
          if(state !== "running") {
              return new Q()
                .then(function () {
                    utils.printDebug("Launching application ..");
                    return dial.launchApplication(host, app);
                })
                .then(function (response) {
                    if(response.statusCode !== 201) {
                        return Q.reject(new Error("Could not launch " + app + " application. Expected status code 201 but got " + response.statusCode));
                    }
                })
                .then(function () {
                    utils.printDebug("Wait for " + timeToWaitForStateChange + " ms for state change to happen");
                })
                .delay(timeToWaitForStateChange)
                .then(function () {
                    utils.printDebug("Querying application state ..");
                })
                .then(dial.getApplicationStatus.bind(null, host, app))
                .then(function getCurrentAppState(result) {
                    if(!result || !result.state) {
                        return Q.reject(new Error("Could not retrieve current " + app + " application state"));
                    }
                    utils.printDebug("Application is in " + result.state + " state");
                    if(result.state !== "running") {
                        return Q.reject(new Error("Expected " + app + " state to be running but state was " + result.state));
                    }
                })
          }
      })

      .then(function () {
          utils.printDebug("Hiding application ..");
      })
      .then(dial.hideApplication.bind(null, host, app))
      .then(function (response) {
          if(response.statusCode !== 200) {
              return Q.reject(new Error("Error hiding " + app + " application. Expected status code 200 but got " + response.statusCode));
          }
      })
      .then(function () {
          utils.printDebug("Wait for " + timeToWaitForStateChange + " ms for state change to happen");
      })
      .delay(timeToWaitForStateChange)
      .then(function () {
          utils.printDebug("Querying application state ..");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          utils.printDebug("Application is in " + result.state + " state");
          if(result.state !== "hidden") {
              return Q.reject(new Error("Expected " + app + " state to be hidden but state was " + result.state));
          }
      })

      .then(function () {
          utils.printTestSuccess()
      })
      .fail(function handleError(err) {
          utils.printTestFailure(err);
      });
}

module.exports.test = test;

if (require.main === module) {
    test()
      .done();
}
