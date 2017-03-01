"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

function test() {
    var host = utils.getParam("host");
    var app = utils.getParam("app");
    var timeToWaitForStateChange = utils.getParam("timeToWaitForStateChange") || 5000;

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Try to stop invalid " + app + " application instance and check for DIAL server response code 404");
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
          console.log("TEST PASSED");
      })
      .fail(function handleError(err) {
          console.error("TEST FAILED " + err);
      });
}

module.exports.test = test;

if (require.main === module) {
    test.done();
}
