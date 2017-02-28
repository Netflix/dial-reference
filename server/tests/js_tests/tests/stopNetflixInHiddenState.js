
"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

function test() {
    var host        = utils.getParam("host");

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Stop Netflix application using DIAL server when the application is in hidden state and expect response code 200");
      })
      .then(dial.getApplicationStatus.bind(null, host, "Netflix"))
      .then(function hideApp(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current Netflix application state"));
          }
          if(result.state !== "hidden") {
              if(!result.dialVer || result.dialVer !== "2.1") { // Hidden state not supported
                  return Q.reject(new Error("This test is only applicable for DIAL version >= 2.1"));
              }

              if(result.state === "stopped") {
                  // Launch and hide app
                  return dial.launchApplication(host, "Netflix")
                      .delay(5000)
                      .then(dial.hideApplication.bind(null, host, "Netflix"))
                      .delay(5000);
              }
              else if(result.state === "starting" || result.state === "running") {
                  // Hide app
                  return dial.hideApplication(host, "Netflix")
                  .delay(5000);
              }
          }
      })
      .then(dial.getApplicationStatus.bind(null, host, "Netflix"))
      .then(function checkAppStatus(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current Netflix application state"));
          }
          if(result.state !== "hidden") {
              return Q.reject(new Error("Expected Netflix app status to be hidden but the status was " + result.state));
          }
      })

      .then(dial.stopApplication.bind(null, host, "Netflix"))
      .then(function (status) {
          if(status !== 200) {
              return Q.reject(new Error("Tried to stop Netflix. Expected statusCode: 200 but got " + status));
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
