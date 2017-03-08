"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

function test() {
    var host = utils.getParam("host");
    var app = utils.getParam("app");

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
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Try to launch " + app + " application with excess payload and ensure DIAL server returns response code 413");
      })
      .then(dial.launchApplication.bind(null, host, app, payload))
      .then(function (response) {
          if(response.statusCode !== 413) {
              return Q.reject(new Error("Tried to launch " + app + " with excess payload. Expected statusCode: 413 but got " + response.statusCode));
          }
      })
      .then(function () {
          utils.printSuccess()
      })
      .fail(function handleError(err) {
          utils.printFailure(err);
      });
}

module.exports.test = test;

if (require.main === module) {
    test()
      .done();
}
