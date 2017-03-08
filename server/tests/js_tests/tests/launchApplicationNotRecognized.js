"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

function test() {
    var host = utils.getParam("host");
    var timeToWaitForStateChange = utils.getParam("timeToWaitForStateChange") || 5000;

    return new Q()
      .then(function () {
          utils.printTestInfo(__filename.slice(__dirname.length + 1), "Launch an application that is not recognized by DIAL server and check for response code 404");
      })
      .then(dial.launchApplication.bind(null, host, "ApplicationNotRecognized"))
      .then(function (response) {
          if(response.statusCode !== 404) {
              return Q.reject(new Error("Tried to launch application that is not recognized by the DIAL server. Expected statusCode: 404 but got " + response.statusCode));
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
