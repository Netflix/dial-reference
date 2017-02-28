"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

function test() {
    var host  = utils.getParam("host");

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Launch Netflix application with payload using DIAL server when the application is in STOPPED state");
      })
      .then(dial.getApplicationStatus.bind(null, host, "Netflix"))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject("Could not retrieve current Netflix application state");
          }
          if(result.state !== "stopped") {
              if(result.allowStop && result.allowStop === "false") {
                  return Q.reject(new Error("The DIAL server does not support STOP operation. Test cannot proceed. Make sure the Netflix app is stopped before re-running the test"));
              }
          }
          return result.state;
      })
      .then(function stopAppIfCurrentlyRunning(state) {
          if(state !== "stopped") {
              return dial.stopApplication(host, "Netflix")
              .then(function (status) {
                  if(status !== 200) {
                      return Q.reject(new Error("Could not stop Netflix application when it was running. Expected status code 200 but got " + status));
                  }
              });
          }
      })
      .delay(5000)

      .then(dial.launchApplication.bind(null, host, "Netflix", "key1=val1&key2=val2"))
      .then(function (status) {
          if(status !== 201) {
              return Q.reject(new Error("Error launching Netflix application. Expected statusCode: 201 but got " + status));
          }
      })
      .delay(5000)
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
