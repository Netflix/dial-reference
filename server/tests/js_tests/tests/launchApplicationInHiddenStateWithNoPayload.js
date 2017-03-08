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
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Launch " + app + " application using DIAL server when app is in hidden state and check for response code 201 ");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function hideApp(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          if(result.state !== "hidden") {
              if(result.state === "stopped") {
                  // Launch and hide app
                  return dial.launchApplication(host, app)
                      .delay(timeToWaitForStateChange)
                      .then(dial.getApplicationStatus.bind(null, host, app))
                      .then(function checkAppStatus(result) {
                          if(!result || !result.state) {
                              return Q.reject(new Error("Error retrieving current " + app + " application state"));
                          }
                          if(result.state !== "running") {
                              return Q.reject(new Error("Expected " + app + " app status to be running but the status was " + result.state));
                          }
                      })
                      .then(dial.hideApplication.bind(null, host, app))
                      .delay(timeToWaitForStateChange);
              }
              else {
                  // Hide app
                  return dial.hideApplication(host, app)
                  .delay(timeToWaitForStateChange);
              }
          }
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function checkAppStatus(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          if(result.state !== "hidden") {
              return Q.reject(new Error("Expected " + app + " app status to be hidden but the status was " + result.state));
          }
      })

      .then(dial.launchApplication.bind(null, host, app))
      .then(function (response) {
          if(response.statusCode !== 201) {
              return Q.reject(new Error("Error launching " + app + " application when it was in hidden state. Expected statusCode: 201 but got " + response.statusCode));
          }
      })
      .delay(timeToWaitForStateChange)
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
