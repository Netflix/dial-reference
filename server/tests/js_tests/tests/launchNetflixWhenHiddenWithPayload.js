"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

function test() {
    var host  = utils.getParam("host");

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + "Launch Netflix application with payload using DIAL server when app is in hidden state and check for response code 201 ");
      })
      .then(dial.getApplicationStatus.bind(null, host, "Netflix"))
      .then(function hideApp(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Error retrieving current Netflix application state"));
          }
          if(result.state !== "hidden") {
              if(result.dialVer && result.dialVer !== "2.1") { // Hidden state not supported
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
              return Q.reject(new Error("Error retrieving current Netflix application state"));
          }
          if(result.state !== "hidden") {
              return Q.reject(new Error("Expected Netflix app status to be hidden but the status was " + result.state));
          }
      })

      .then(dial.launchApplication.bind(null, host, "Netflix", "key1=val1&key2=val2"))
      .then(function (status) {
          if(status !== 201) {
              return Q.reject(new Error("Error launching Netflix application when it was in hidden state. Expected statusCode: 201 but got " + status));
          }
      })
      .delay(5000) //Allow time for app to launch
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
