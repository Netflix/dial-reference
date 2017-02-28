"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

function test() {
    var host = utils.getParam("host");

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Try to stop invalid Netflix application instance and check for DIAL server response code 404");
      })
      .then(dial.launchApplication.bind(null, host, "Netflix"))
      .delay(5000)
      .then(function getCurrentAppState() {
          return dial.getApplicationStatus(host, "Netflix")
              .then(function (result) {
                  if(!result || !result.state) {
                      return Q.reject(new Error("Error retrieving current Netflix application state"));
                  }
                  if(result.allowStop && result.allowStop === "false") {
                      return Q.reject(new Error("This test is not applicable for servers that do not support DIAL stop operation."));
                  }
                  if(result.state !== "running") {
                      return Q.reject(new Error("Expected Netflix application to be running but server returned application state as " + result.state));
                  }
              });
      })
      .delay(5000)
      .then(function () {
          return dial.getAppsUrl(host);
      })
      .then(function (appsUrl) {
          var invalidInstanceUrl =  appsUrl + "/Netflix/xyz";
          return invalidInstanceUrl;
      })
      .then(function (url) {
          return dial.stopApplicationInstance(url);
      })
      .then(function (status) {
          if(status !== 404) {
              return Q.reject(new Error("Tried to stop invalid Netflix application instance. Expected statusCode: 404 but got " + status));
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
