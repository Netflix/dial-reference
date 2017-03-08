"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

function test() {
    var host = utils.getParam("host");
    var app = utils.getParam("app");
    var timeToWaitForStateChange = utils.getParam("timeToWaitForStateChange") || 5000;

    return new Q()
      .then(function () {
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Stop " + app + " application using DIAL server when the application is in hidden state and expect response code 200");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function hideApp(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          if(result.state !== "hidden") {
              if(result.state === "stopped") {
                  // Launch and hide app
                  return dial.launchApplication(host, app)
                      .delay(timeToWaitForStateChange)
                      .then(dial.getApplicationStatus.bind(null, host, app))
                      .then(function getAppState(result) {
                          if(!result || !result.state) {
                              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
                          }
                          if(result.state !== "running") {
                              return Q.reject(new Error("Expected " + app + " state to be running but state was " + result.state));
                          }
                      })
                      .then(dial.hideApplication.bind(null, host, app))
                      .delay(timeToWaitForStateChange)

              }
              return dial.hideApplication(host, app)
                .delay(timeToWaitForStateChange);
          }
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function checkAppStatus(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          if(result.state !== "hidden") {
              return Q.reject(new Error("Expected " + app + " app state to be hidden but the state was " + result.state));
          }
      })

      .then(dial.stopApplication.bind(null, host, app))
      .then(function (response) {
          if(response.statusCode !== 200) {
              return Q.reject(new Error("Tried to stop " + app + ". Expected statusCode: 200 but got " + response.statusCode));
          }
      })
      .delay(timeToWaitForStateChange)
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function checkAppStatus(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current " + app + " application state"));
          }
          if(result.state !== "stopped") {
              return Q.reject(new Error("Expected " + app + " app state to be stopped but the state was " + result.state));
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
