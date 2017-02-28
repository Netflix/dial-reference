"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

function test() {
    var host = utils.getParam("host");

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Launch an invalid application using DIAL server and check for response code 404");
      })
      .then(dial.launchApplication.bind(null, host, "InvalidApplication"))
      .then(function (status) {
          if(status !== 404) {
              return Q.reject(new Error("Tried to launch invalid application using DIAL server. Expected statusCode: 404 but got " + status));
          }
      })
      .then(dial.launchApplication.bind(null, host, "InvalidApplication", "key1=val1&key2=val2"))
      .then(function (status) {
          if(status !== 404) {
              return Q.reject(new Error("Tried to launch invalid application with a payload using DIAL server. Expected statusCode: 404 but got " + status));
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
