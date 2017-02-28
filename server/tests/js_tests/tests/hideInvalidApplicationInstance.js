"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

function test() {
    var host = utils.getParam("host");

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Try to hide an invalid instance of Netflix application and expect status code 404");
      })
      .then(dial.launchApplication.bind(null, host, "Netflix"))
      .then(function (status) {
          if(status !== 200 && status !== 201) {
              return Q.reject(new Error("Error launching Netflix application. Expected status code 200/201 but got " + status));
          }
      })
      .then(function () {
          return dial.getApplicationStatus(host, "Netflix");
      })
      .then(function getCurrentAppState(result) {
          if(!result || !result.state) {
              return Q.reject(new Error("Could not retrieve current Netflix application state"));
          }
          if(result.dialVer && result.dialVer !== "2.1") {
              return Q.reject(new Error("This test is only applicable for DIAL version >= 2.1"));
          }
          return result.state;
      })
      .then(function () {
          return dial.getAppsUrl(host);
      })
      .then(function (appsUrl) {
          var invalidInstanceUrl =  appsUrl + "/application/xyz";
          return invalidInstanceUrl;
      })
      .then(function (url) {
          return dial.hideApplicationInstance(url);
      })
      .then(function (status) {
          if(status !== 404) {
              return Q.reject(new Error("Tried to hide invalid application instance. Expected statusCode: 404 but got " + status));
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
