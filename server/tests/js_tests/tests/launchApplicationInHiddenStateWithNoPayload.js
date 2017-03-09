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
        describe: "Time(ms) to wait between state changes before querying application status",
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
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Launch " + app + " application using DIAL server when app is in hidden state and check for response code 201 ");
      })
      .then(function () {
          utils.printDebug("Querying application status ..");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function hideApp(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          utils.printDebug("Application is in " + result.state + " state");
          if(result.state !== "hidden") {
              if(result.state === "stopped") {
                  // Launch and hide app
                  return new Q()
                      .then(function () {
                          utils.printDebug("Launching application ..");
                          return dial.launchApplication(host, app);
                      })
                      .then(function () {
                          utils.printDebug("Wait for " + timeToWaitForStateChange + " ms for state change to happen");
                      })
                      .delay(timeToWaitForStateChange)
                      .then(function () {
                          utils.printDebug("Querying application status ..");
                      })
                      .then(dial.getApplicationStatus.bind(null, host, app))
                      .then(function checkAppStatus(result) {
                          if(!result || !result.state) {
                              return Q.reject(new Error("Error retrieving current " + app + " application state"));
                          }
                          utils.printDebug("Application is in " + result.state + " state");
                          if(result.state !== "running") {
                              return Q.reject(new Error("Expected " + app + " app status to be running but the status was " + result.state));
                          }
                      })
                      .then(function () {
                          utils.printDebug("Hiding application ..");
                      })
                      .then(dial.hideApplication.bind(null, host, app))
                      .then(function () {
                          utils.printDebug("Wait for " + timeToWaitForStateChange + " ms for state change to happen");
                      })
                      .delay(timeToWaitForStateChange)
                      .then(function () {
                          utils.printDebug("Querying application state ..");
                          return dial.getApplicationStatus(host, app)
                      })
                      .then(function checkAppStatus(result) {
                          if(!result || !result.state) {
                              return Q.reject(new Error("Error retrieving current " + app + " application state"));
                          }
                          utils.printDebug("Application is in " + result.state + " state");
                          if(result.state !== "hidden") {
                              return Q.reject(new Error("Expected " + app + " app status to be hidden but the status was " + result.state));
                          }
                      });
              }
              // Hide app
              return new Q()
                .then(function () {
                    utils.printDebug("Hiding application ..");
                    return dial.hideApplication(host, app);
                })
                .then(function () {
                    utils.printDebug("Wait for " + timeToWaitForStateChange + " ms for state change to happen");
                })
                .delay(timeToWaitForStateChange)
                .then(function () {
                    utils.printDebug("Querying application state ..");
                    return dial.getApplicationStatus(host, app)
                })
                .then(function checkAppStatus(result) {
                    if(!result || !result.state) {
                        return Q.reject(new Error("Error retrieving current " + app + " application state"));
                    }
                    utils.printDebug("Application is in " + result.state + " state");
                    if(result.state !== "hidden") {
                        return Q.reject(new Error("Expected " + app + " app status to be hidden but the status was " + result.state));
                    }
                });
          }
      })

      .then(function () {
          utils.printDebug("Launching application ..");
      })
      .then(dial.launchApplication.bind(null, host, app))
      .then(function (response) {
          if(response.statusCode !== 201) {
              return Q.reject(new Error("Error launching " + app + " application when it was in hidden state. Expected statusCode: 201 but got " + response.statusCode));
          }
      })
      .then(function () {
          utils.printDebug("Wait for " + timeToWaitForStateChange + " ms for state change to happen");
      })
      .delay(timeToWaitForStateChange)
      .then(function () {
          utils.printDebug("Querying application state ..");
          return dial.getApplicationStatus(host, app)
      })
      .then(function checkAppStatus(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          utils.printDebug("Application is in " + result.state + " state");
          if(result.state !== "running") {
              return Q.reject(new Error("Expected " + app + " app status to be running but the status was " + result.state));
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
