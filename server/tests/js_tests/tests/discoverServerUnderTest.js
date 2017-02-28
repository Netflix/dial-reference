"use strict";

var dial      =   require("../libs/dialClient.js"),
    utils     =   require("../libs/utils.js"),
    Q         =   require("q");

function test() {
    var testServer = utils.getParam("host");

    return new Q()
      .then(function () {
          console.log("TEST " + __filename + ": Perform DIAL discovery and ensure that the server under test is discovered");
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
          console.log("TEST PASSED");
      })
      .fail(function handleError(err) {
          console.log("TEST FAILED " + err);
      });
}

module.exports.test = test;

if (require.main === module) {
    test.done();
}
