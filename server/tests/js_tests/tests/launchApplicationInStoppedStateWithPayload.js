"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

function test() {
    var host  = utils.getParam("host");
    var app = utils.getParam("app");
    var timeToWaitForStateChange = utils.getParam("timeToWaitForStateChange") || 5000;

    return new Q()
      .then(function () {
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Launch " + app + " application with payload using DIAL server when the application is in STOPPED state and expect response code 201");
      })
      .then(dial.getApplicationStatus.bind(null, host, app))
      .then(function stopAppIfNecessary(result) {
          if(!result || !result.state) {
              return Q.reject("Error retrieving " + app + " application state");
          }
          if(result.state !== "stopped") {
              if(!result.href) {
                  return Q.reject(new Error("Unable to to retrieve href attribute from application status. This means the DIAL server does not support STOP operation. " +
                      "Test cannot proceed. Stop the " + app + " app manually before re-running this test"));
              }
              return dial.stopApplication(host, app)
              .then(function (response) {
                  if(response.statusCode !== 200) {
                      return Q.reject(new Error("Could not stop " + app + " application when it was in " + result.state + " state. Expected status code 200 but got " + response.statusCode));
                  }
              })
              .delay(timeToWaitForStateChange)
              .then(dial.getApplicationStatus.bind(null, host, app))
              .then(function checkAppState(result) {
                  if(!result || !result.state) {
                      return Q.reject("Error retrieving " + app + " application state");
                  }
                  if(result.state !== "stopped") {
                      return Q.reject(new Error("Expected " + app + " application state to be stopped but querying for state returned " + result.state));
                  }
              });
          }
          return result.state;
      })

      .then(dial.launchApplication.bind(null, host, app, "key1=val1"))
      .then(function (response) {
          if(response.statusCode !== 201) {
              return Q.reject(new Error("Error launching " + app + " application. Expected statusCode: 201 but got " + response.statusCode));
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
