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
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Try to hide an invalid instance of " + app + " application and expect status code 404");
      })
      .then(dial.launchApplication.bind(null, host, app))
      .then(function (response) {
          if(response.statusCode !== 201) {
              return Q.reject(new Error("Error launching " + app + " application. Expected status code 201 but got " + response.statusCode));
          }
      })
      .delay(timeToWaitForStateChange)
      .then(function () {
          return dial.getApplicationStatus(host, app);
      })
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          if(result.state !== "running") {
              return Q.reject(new Error("Expected " + app + " state to be running but state was " + result.state));
          }
      })
      .then(function () {
          return dial.getAppsUrl(host);
      })
      .then(function (appsUrl) {
          var invalidInstanceUrl =  appsUrl + "/" + app + "/xyz";
          return invalidInstanceUrl;
      })
      .then(function (url) {
          return dial.hideApplicationInstance(url);
      })
      .then(function (response) {
          if(response.statusCode === 501) {
              return Q.reject(new Error("The DIAL server returned 501 NOT IMPLEMENTED. This means it does not support HIDE operation"));
          }
          if(response.statusCode !== 404) {
              return Q.reject(new Error("Tried to hide invalid application instance. Expected statusCode: 404 but got " + response.statusCode));
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
