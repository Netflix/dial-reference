"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

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
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Try to stop invalid " + app + " application instance and check for DIAL server response code 404");
      })
      .then(dial.launchApplication.bind(null, host, app))
      .delay(timeToWaitForStateChange)
      .then(function getCurrentAppState() {
          return dial.getApplicationStatus(host, app)
              .then(function (result) {
                  if(!result || !result.state) {
                      return Q.reject(new Error("Error retrieving current " + app + " application state"));
                  }
                  if(result.allowStop && result.allowStop === "false") {
                      return Q.reject(new Error("This test is not applicable for servers that do not support DIAL stop operation."));
                  }
                  if(result.state !== "running") {
                      return Q.reject(new Error("Expected " + app + " application to be running but server returned application state as " + result.state));
                  }
              });
      })
      .delay(timeToWaitForStateChange)
      .then(function () {
          return dial.getAppsUrl(host);
      })
      .then(function (appsUrl) {
          var invalidInstanceUrl =  appsUrl + "/" + app + "/xyz";
          return invalidInstanceUrl;
      })
      .then(function (url) {
          return dial.stopApplicationInstance(url);
      })
      .then(function (response) {
          if(response.statusCode !== 404) {
              return Q.reject(new Error("Tried to stop invalid " + app + " application instance. Expected statusCode: 404 but got " + response.statusCode));
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
