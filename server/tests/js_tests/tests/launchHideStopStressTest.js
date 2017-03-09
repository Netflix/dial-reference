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
    var timeToWaitForStateChange = argv.timeToWaitForStateChange || 5000;
    var maxCount = 100;
    var count = 0;


    utils.printTestInfo("TEST " + __filename.slice(__dirname.length + 1), "Launch, hide and stop app cycle - stress testing");

    function cycle() {
        return new Q()
          // Launch app
          .then(function startApp(state) {
              utils.printDebug("Launching application ..");
              return dial.launchApplication(host, app)
                .then(function (response) {
                    if(response.statusCode !== 201) {
                        return Q.reject(new Error("Error launching " + app + " application. Expected status code 201 from DIAL server but got " + response.statusCode));
                    }
                });
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
                  return Q.reject(new Error("Error retrieving current " + app + " application state"));
              }
              utils.printDebug("Application is in " + result.state + " state");
              if(result.state !== "running") {
                  return Q.reject(new Error("Expected " + app + " application to be in running state, but querying state returned state as" + result.state));
              }
          })

          // Hide app
          .then(function hideApp(state) {
              utils.printDebug("Hiding application ..");
              return dial.hideApplication(host, app)
                .then(function (response) {
                    if(response.statusCode !== 200) {
                        return Q.reject(new Error("Error hiding " + app + " application. Expected status code 200 from DIAL server but got " + response.statusCode));
                    }
                });
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
                  return Q.reject(new Error("Error retrieving current " + app + " application state"));
              }
              utils.printDebug("Application is in " + result.state + " state");
              if(result.state !== "hidden") {
                  return Q.reject(new Error("Expected " + app + " application to be in hidden state, but querying state returned state as" + result.state));
              }
          })

          // Stop app
          .then(function stopApp(state) {
              return dial.stopApplication(host, app)
                .then(function (response) {
                    if(response.statusCode !== 200) {
                        return Q.reject(new Error("Error stopping " + app + " application. Expected status code 200 from DIAL server but got " + response.statusCode));
                    }
                });
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
                  return Q.reject(new Error("Error retrieving current " + app + " application state"));
              }
              utils.printDebug("Application is in " + result.state + " state");
              if(result.state !== "stopped") {
                  return Q.reject(new Error("Expected " + app + " application to be in stopped state, but querying state returned state as" + result.state));
              }
          })

          .then(function () {
              count++;
              utils.printInfo("Executed cycle " + count + " times");
              if(count === maxCount) {
                  utils.printInfo("TEST END");
              }
              else {
                  return cycle();
              }
          })
    }

    return cycle();
}

module.exports.test = test;

if (require.main === module) {
    test()
      .done();
}
