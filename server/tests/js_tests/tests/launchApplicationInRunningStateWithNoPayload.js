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
          console.log("TEST " + __filename + ": Launch " + app + " without payload using DIAL server when application is already running and check for response code 201");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          return result.state;
      })

      .then(function startAppIfNotRunning(state) {
          if(state !== "running") {
              return dial.launchApplication(host, app)
                .then(function (response) {
                    if(response.statusCode !== 201) {
                        return Q.reject(new Error("Error launching " + app + " application. Expected status code 201 from DIAL server but got " + response.statusCode));
                    }
                });
          }
      })
      .delay(timeToWaitForStateChange)
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          if(result.state !== "running") {
              return Q.reject(new Error("Expected " + app + " application to be in running state, but querying state returned state as" + result.state));
          }
      })

      .then(dial.launchApplication.bind(null, host, app))
      .then(function (response) {
          if(response.statusCode !== 201) {
              return Q.reject(new Error("Error launching " + app + " application when it was already running. Expected status code 201 from DIAL server but got " + response.statusCode));
          }
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current " + app + " application state"));
          }
          if(result.state !== "running") {
              return Q.reject(new Error("Expected " + app + " application to be in running state, but querying state returned state as" + result.state));
          }
      })
      .then(function () {
          console.log("TEST PASSED");
      })
      .fail(function handleError(err) {
          console.error("TEST FAILED " + err);
      });
}

module.exports.test = test;

if (require.main === module) {
    test()
      .done();
}
