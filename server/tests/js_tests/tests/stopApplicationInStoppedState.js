"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

function test() {
    var host = utils.getParam("host");
    var app = utils.getParam("app");
    var timeToWaitForStateChange = utils.getParam("timeToWaitForStateChange") || 5000;
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
          .then(dial.getApplicationStatus.bind(null, host, app))
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
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          if(result.state !== "stopped") {
              return Q.reject(new Error("Expected " + app + " application state to be stopped but state was " + result.state));
          }
      })
      .then(function () {
          utils.printSuccess()
      })
      .fail(function handleError(err) {
          utils.printFailure(err);
      });
}

module.exports.test = test;

if (require.main === module) {
    test()
      .done();
}
