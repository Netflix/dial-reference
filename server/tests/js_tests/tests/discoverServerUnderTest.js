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
    var testServer = argv.host;

    return new Q()
      .then(function () {
          utils.printTestInfo(__filename.slice(__dirname.length + 1) , "Perform DIAL discovery and ensure that the server under test is discovered");
      })
      .then(function discover() {
          utils.printDebug("Performing discovery ..");
          return dial.discover();
      })
      .then(function findServerInList(servers) {
          var found = false;
          servers.forEach(function (server) {
              if(server.host === testServer) {
                  utils.printDebug("Found " + server.host + " in discovered list of servers");
                  found = true;
              }
          });
          if(!found) {
              utils.printDebug("Did not find " + testServer + " in discovered list of servers");
              return Q.reject(new Error("DIAL client was not able to discover the server under test : " + testServer));
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
