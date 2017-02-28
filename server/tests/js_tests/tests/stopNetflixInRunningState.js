"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

var host = utils.getParam("host");

function test() {
    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Stop Netflix application when it is running and check for response code 200 from DIAL server ");
      })
      .then(dial.getApplicationStatus.bind(null, host, "Netflix"))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current Netflix application state"));
          }
          if(result.allowStop && result.allowStop === "false") {
              return Q.reject(new Error("This test is not applicable for servers that do not support DIAL stop operation."));
          }
          return result.state;
      })

      .then(function startAppIfNotRunning(state) {
          if(state !== "starting" && state !== "running") {
              return dial.launchApplication(host, "Netflix")
                .then(function (status) {
                    if(status !== 201) {
                        return Q.reject("Error launching Netflix application. Expected status code 201 but got " + status);
                    }
                });
          }
      })
      .delay(5000)
      .then(dial.stopApplication.bind(null, host, "Netflix"))
      .then(function (status) {
          if(status !== 200) {
              return Q.reject("Error stopping Netflix application. Expected status code 200 but got " + status);
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
