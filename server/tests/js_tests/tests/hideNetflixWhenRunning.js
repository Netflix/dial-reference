"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

function test() {
    var host        = utils.getParam("host");

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Hide netflix application when it is running and expect response code 200");
      })
      .then(dial.getApplicationStatus.bind(null, host, "Netflix"))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current Netflix application state"));
          }
          if(result.dialVer && result.dialVer !== "2.1") {
              return Q.reject(new Error("This test is only applicable for DIAL version >= 2.1"));
          }
          return result.state;
      })

      .then(function startAppIfNotRunning(state) {
          if(state !== "running") {
              return dial.launchApplication(host, "Netflix")
                .then(function (status) {
                    if(status !== 201) {
                        return Q.reject(new Error("Could not launch Netflix application. Expected status code 201 but got " + status));
                    }
                });
          }
      })
      .delay(5000) // Allow time for app to start

      .then(dial.hideApplication.bind(null, host, "Netflix"))
      .then(function (status) {
          if(status !== 200) {
              return Q.reject(new Error("Error stopping Netflix application. Expected status code 200 but got " + status));
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
