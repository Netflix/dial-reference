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
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Try to hide " + app + " when it is already in hidden state and expect response code 200 from the DIAL server");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function hideApp(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          utils.printDebug("Application status is " + result.state);
          if(result.state !== "hidden") {
              if(result.state === "stopped") {
                  // Launch and hide app
                  return dial.launchApplication(host, app)
                      .then(function () {
                          utils.printDebug("Requested server to launch application ..");
                      })
                      .delay(timeToWaitForStateChange)
                      .then(dial.getApplicationStatus.bind(null, host, app))
                      .then(function checkAppStatus(result) {
                          if(!result || !result.state) {
                              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
                          }
                          utils.printDebug("Application status is " + result.state);
                          if(result.state !== "running") {
                              return Q.reject(new Error("Expected " + app + " app status to be running but the state was " + result.state));
                          }
                      })
                      .then(function () {
                          utils.printDebug("Hide application ..");
                      })
                      .then(dial.hideApplication.bind(null, host, app))
                      .delay(timeToWaitForStateChange);
              }
              return new Q()
                .then(function () {
                    utils.printDebug("Hide application ..");
                    return dial.hideApplication(host, app);
                })
                .delay(timeToWaitForStateChange);
          }
      })
      .then(function () {
          return dial.getApplicationStatus(host, app)
      })
      .then(function checkAppStatus(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          utils.printDebug("Application status is now " + result.state);
          if(result.state !== "hidden") {
              return Q.reject(new Error("Expected " + app + " app status to be hidden but the state was " + result.state));
          }
      })

      .then(function () {
          utils.printDebug("Hide application ..");
      })
      .then(dial.hideApplication.bind(null, host, app))
      .then(function (response) {
          if(response.statusCode !== 200) {
              return Q.reject(new Error("Tried to hide " + app + ". Expected statusCode: 200 but got " + response.statusCode));
          }
      })
      .delay(timeToWaitForStateChange)
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function checkAppStatus(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          utils.printDebug("Application status is " + result.state);
          if(result.state !== "hidden") {
              return Q.reject(new Error("Expected " + app + " app status to be hidden but the state was " + result.state));
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
