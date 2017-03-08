"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

const argv    = require("yargs")
    .usage("\nUsage: node " + __filename.slice(__dirname.length + 1) + "[options]")
    .option("host", {
        describe: "IP address of host on which DIAL server under test is running",
        type: "string",
        demand: true
    })
    .option("application", {
        alias: "app",
        describe: "Application to test",
        type: "string",
        demand: true
    })
    .help("help").alias("help", "h").argv;

function test() {
    var host = argv.host;
    var app = argv.application;

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
          utils.printTestSuccess()
      })
      .fail(function handleError(err) {
          utils.printTestFailure(err);
      });
}

module.exports.test = test;

if (require.main === module) {
    test()
      .done();
}
