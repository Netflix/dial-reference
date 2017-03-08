"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

function test() {
    var testServer = utils.getParam("host");

    return new Q()
      .then(function () {
          utils.printTestInfo(__filename.slice(__dirname.length + 1) , "Perform DIAL discovery and ensure that the server under test is discovered");
      })
      .then(dial.discover)
      .then(function findServerInList(servers) {
          var found = false;
          servers.forEach(function (server) {
              if(server.host === testServer) {
                  found = true;
              }
          });
          if(!found) {
              return Q.reject(new Error("DIAL client was not able to discover the server under test : " + testServer));
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
