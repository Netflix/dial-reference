"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

function test() {
    var host = utils.getParam("host");

    var payload = "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&"
     + "key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value&key=value";

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Try to launch Netflix app with excess payload and ensure DIAL server returns response code 413");
      })
      .then(dial.launchApplication.bind(null, host, "Netflix", payload))
      .then(function (status) {
          if(status !== 413) {
              return Q.reject(new Error("Tried to launch Netflix with excess payload. Expected statusCode: 413 but got " + status));
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
