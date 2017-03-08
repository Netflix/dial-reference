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
    var instanceUrl;

    return new Q()
      .then(function () {
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Try to stop " + app + " application using DIAL server when the application is already stopped and expect response code 200.");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }

          if(result.allowStop && result.allowStop === "false") {
              return Q.reject(new Error("This test is not applicable for DIAL servers that do not support STOP operation"));
          }
          return result.state;
      })
      .then(function getRunningInstanceUrlAndStopApp(state) {
          if(state !== "stopped") {
              return new Q()
              .then(dial.constructAppResourceUrl.bind(null, host, app))
              .then(function getInstanceUrl(appResourceUrl) {
                  return dial.getApplicationStatus(host, app)
                    .then(function (response) {
                        if(response.href) {
                            instanceUrl = appResourceUrl + "/" + response.href; // Construct Application Instance Url
                            return instanceUrl;
                        }
                        return Q.reject(new Error("Could not get instance href from application status to construct Application Instance Url"));
                    });
              });
          }
          return new Q()
          .then(dial.launchApplication.bind(null, host,  app))
          .delay(timeToWaitForStateChange)
          .then(function () {
              return dial.getApplicationStatus(host, app)
          })
          .then(function getCurrentAppState(result) {
              if(!result || !result.state) {
                  return Q.reject(new Error("Could not retrieve current " + app + " application state"));
              }
              if(result.state !== "running") {
                  return Q.reject(new Error("Expected " + app + " application state to be running but state was " + result.state));
              }
          })
          .then(function () {
              return dial.constructAppResourceUrl(host, app);
          })
          .then(function getInstanceUrl(appResourceUrl) {
              return dial.getApplicationStatus(host, app)
                .then(function (response) {
                    if(response.href) {
                        instanceUrl = appResourceUrl + "/" + response.href; // Construct Application Instance Url
                        return instanceUrl;
                    }
                    return Q.reject(new Error("Could not get instance href from application status to construct Application Instance Url"));
                });
          });
      })

      .then(function stopApp() {
          return dial.stopApplicationInstance(instanceUrl);
      })
      .then(function (response) {
          if(response.statusCode !== 200) {
              return Q.reject(new Error("Could not stop " + app + " application when it was running. Expected status code 200 but got " + response.statusCode));
          }
      })
      .delay(timeToWaitForStateChange)
      .then(function () {
          return dial.getApplicationStatus(host, app)
      })
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          if(result.state !== "stopped") {
              return Q.reject(new Error("Expected " + app + " application state to be stopped but state was " + result.state));
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
