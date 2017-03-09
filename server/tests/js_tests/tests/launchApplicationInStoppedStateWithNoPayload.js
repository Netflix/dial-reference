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
    var host  = argv.host;
    var app = argv.application;
    var timeToWaitForStateChange = argv.timeToWaitForStateChange || 5000;

    return new Q()
      .then(function () {
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Launch " + app + " application using DIAL server when the application is in STOPPED state and expect response code 201");
      })
      .then(function () {
          utils.printDebug("Querying application state ..");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function stopAppIfNecessary(result) {
          if(!result || !result.state) {
              return Q.reject("Error retrieving " + app + " application state");
          }
          utils.printDebug("Application is in " + result.state + " state");
          if(result.state !== "stopped") {
              if(!result.href) {
                  return Q.reject(new Error("Unable to to retrive href attribute from application status. This means the DIAL server does not support STOP operation. " +
                      "Test cannot proceed. Stop the " + app + " app manually before re-running this test"));
              }
              return new Q()
              .then(function () {
                  utils.printDebug("Stopping application ..");
                  return dial.stopApplication(host, app);
              })
              .then(function (response) {
                  if(response.statusCode !== 200) {
                      return Q.reject(new Error("Could not stop " + app + " application when it was in " + result.state + " state. Expected status code 200 but got " + response.statusCode));
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
              .then(function checkAppState(result) {
                  if(!result || !result.state) {
                      return Q.reject("Error retrieving " + app + " application state");
                  }
                  utils.printDebug("Application is in " + result.state + " state");
                  if(result.state !== "stopped") {
                      return Q.reject(new Error("Expected " + app + " application state to be stopped but querying for state returned " + result.state));
                  }
              });
          }
          return result.state;
      })

      .then(function () {
          utils.printDebug("Launching application ..");
      })
      .then(dial.launchApplication.bind(null, host, app))
      .then(function (response) {
          if(response.statusCode !== 201) {
              return Q.reject(new Error("Error launching " + app + " application. Expected statusCode: 201 but got " + response.statusCode));
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
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          utils.printDebug("Application is in " + result.state + " state");
          if(result.state !== "running") {
              return Q.reject(new Error("Expected " + app + " application to be in running state, but querying state returned state as" + result.state));
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
