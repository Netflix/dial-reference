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
    .help("help").alias("help", "h").argv;

function test() {
    var host = argv.host;
    var app = argv.application;

    return new Q()
      .then(function () {
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Launch/Hide/Stop " + app + " without DIAL server and query application states");
      })
      .then(function () {
          utils.printDebug("Querying application status ..");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function (result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          utils.printDebug("Application is in " + result.state + " state");
      })

      .then(function stopApp() {
          return utils.ask("If application is running, stop it and press Enter once the application is stopped");
      })
      .then(function () {
          utils.printDebug("Querying application status ..");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function (result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          utils.printDebug("Application is in " + result.state + " state");
          if(result.state !== "stopped") {
              return Q.reject(new Error("Application was expected to be in stopped state but was in " + result.state + " state"));
          }
      })

      .then(function launchApp() {
          return utils.ask("Launch the application manually and press Enter when the application is launched");
      })
      .then(function () {
          utils.printDebug("Querying application status ..");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function (result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          utils.printDebug("Application is in " + result.state + " state");
          if(result.state !== "running") {
              return Q.reject(new Error("Application was expected to be in running state but was in " + result.state + " state"));
          }
      })

      .then(function hideApp() {
          return utils.ask("Hide the application manually and press Enter when the application is launched");
      })
      .then(function () {
          utils.printDebug("Querying application status ..");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function (result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          utils.printDebug("Application is in " + result.state + " state");
          if(result.state !== "hidden") {
              return Q.reject(new Error("Application was expected to be in hidden state but was in " + result.state + " state"));
          }
      })

      .then(function stopApp() {
          return utils.ask("Stop the application manually and press Enter when the application is launched");
      })
      .then(function () {
          utils.printDebug("Querying application status ..");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function (result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          utils.printDebug("Application is in " + result.state + " state");
          if(result.state !== "stopped") {
              return Q.reject(new Error("Application was expected to be in stopped state but was in " + result.state + " state"));
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
