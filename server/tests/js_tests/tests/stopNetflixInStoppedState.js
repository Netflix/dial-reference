"use strict";

var dial              =   require("../libs/dialClient.js"),
    utils             =   require("../libs/utils.js"),
    Q                 =   require("q");

function test() {
    var host = utils.getParam("host");
    var instanceUrl;

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Try to stop Netflix application using DIAL server when the application is already stopped and expect response code 200.");
      })
      .then(dial.getApplicationStatus.bind(null, host, "Netflix"))
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current Netflix application state"));
          }
          if(result.state !== "stopped") {
              if(result.allowStop && result.allowStop === "false") {
                  return Q.reject(new Error("The DIAL server does not support STOP operation. Test cannot proceed. Make sure the Netflix app is stopped before re-running the test"));
              }
          }
          return result.state;
      })
      .then(function getRunningInstanceUrlAndStopApp(state) {
          if(state !== "stopped") {
              return new Q()
              .then(dial.constructAppResourceUrl.bind(null, host, "Netflix"))
              .then(function getInstanceUrl(appResourceUrl) {
                  return dial.getApplicationStatus(host, "Netflix")
                    .then(function (response) {
                        if(response.href) {
                            instanceUrl = appResourceUrl + "/" + response.href; // Construct Application Instance Url
                            return instanceUrl;
                        }
                        return Q.reject(new Error("Could not get instance href from application status to construct Application Instance Url"));
                    });
              });
          }
          return new Q()
          .then(dial.launchApplication.bind(null, host,  "Netflix"))
          .delay(5000)
          .then(function () {
              return dial.constructAppResourceUrl(host, "Netflix");
          })
          .then(function getInstanceUrl(appResourceUrl) {
              return dial.getApplicationStatus(host, "Netflix")
                .then(function (response) {
                    if(response.href) {
                        instanceUrl = appResourceUrl + "/" + response.href; // Construct Application Instance Url
                        return instanceUrl;
                    }
                    return Q.reject(new Error("Could not get instance href from application status to construct Application Instance Url"));
                });
          });
      })

      .then(function stopApp() {
          return dial.stopApplicationInstance(instanceUrl);
      })
      .then(function (status) {
          if(status !== 200) {
              return Q.reject(new Error("Could not stop Netflix application when it was running. Expected status code 200 but got " + status));
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
