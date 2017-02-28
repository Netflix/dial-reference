"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

function test() {
    var host  = utils.getParam("host");

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Launch Netflix with payload using DIAL server when application is already running and check for response code 200/201");
      })
      .then(dial.getApplicationStatus.bind(null, host, "Netflix"))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current Netflix application state"));
          }
          return result.state;
      })

      .then(function startAppIfNotRunning(state) {
          if(state !== "starting" || state !== "running") {
              return dial.launchApplication(host, "Netflix")
                .then(function (status) {
                    if(status !== 201) {
                        return Q.reject(new Error("Error launching Netflix application. Expected status code 201 from DIAL server but got " + status));
                    }
                });
          }
      })
      .delay(5000)

      .then(dial.launchApplication.bind(null, host, "Netflix", "key1=val1&key2=val2"))
      .then(function (status) {
          if(status !== 201 && status !== 200) {
              return Q.reject(new Error("Error launching Netflix application when it was already running. Expected status code 200/201 from DIAL server but got " + status));
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
