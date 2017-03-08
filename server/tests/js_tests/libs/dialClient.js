"use strict";

var ssdp        = require("node-ssdp"),
    httpRequest = require("request"),
    Q           = require("q");

var Client = ssdp.Client,
    client = new Client();

var servers = []; // Persist discovered servers

function discover() {
    servers = []; // Re-initialize persisted servers
    var dialServers = {};
    // On M-Search response
    client.on("response", function (headers) {
        if(dialServers[headers.USN] === undefined) { //Resolve duplicates by USN
            dialServers[headers.USN] = {
                location : headers.LOCATION,
                host : headers.LOCATION.split("http://")[1].split(":")[0],
                ST : headers.ST
            };
            if(headers.WAKEUP) {
                var wakeup = headers.WAKEUP;
                dialServers[headers.USN].mac = wakeup.split(";")[0].split("=")[1];
                dialServers[headers.USN].timeout = wakeup.split(";")[1].split("=")[1];
            }

            servers.push(dialServers[headers.USN]);
        }
    });

    // Send M-Search request
    client.search("urn:dial-multiscreen-org:service:dial:1");

    return new Q.Promise(function (resolve) {
        setTimeout(function () {
            client.stop();
            return resolve(servers);
        }, 5000);
    });
}

function getApplicationStatus(host, app, clientDialVer) {
    clientDialVer = clientDialVer || "2.1"; // To test backward compatibility, 2.1 by default

    return new Q()
      .then(constructAppResourceUrl.bind(null, host, app))
      .then(function (appResourceUrl) {
          appResourceUrl += clientDialVer === "2.1" ? "?clientDialVer=2.1" : "";
          return appResourceUrl;
      })
      .then(function (appResourceUrl) {
          return new Q.Promise(function (resolve, reject) {
              // Get application status
              return httpRequest({
                  url: appResourceUrl,
                  method: "GET",
                  timeout: 6000
              }, function handleResponse(error, response, body) {
                  if(!error) {
                      // Check for correct header and status code
                      var statusCode = response.statusCode;
                      var contentType = response.headers["content-type"];
                      if(statusCode !== 200) {
                          return reject(new Error("Expected statusCode 200 while querying application status but got " + statusCode));
                      }
                      if(contentType.indexOf("text/xml") === -1) {
                          return reject(new Error("Expected MIME type 'text/xml' while querying application status but got " + contentType));
                      }
                      // Extract fields from body
                      try {
                          if(body.indexOf("xmlns") === -1 || body.indexOf("<name>") === -1 || body.indexOf("<state>") === -1) {
                              return reject(new Error("One or more required fields were not present in the application status response"));
                          }
                          var parsedResponse = {
                              "xmlns" : body.split("xmlns=")[1].split(" ")[0].split(">")[0].replace(/\"/g, ""),
                              "name" : body.split("<name>")[1].split("</name>")[0].replace(/\s/g, ""),
                              "state" : body.split("<state>")[1].split("</state>")[0].replace(/\s/g, "")
                          };
                          if(parsedResponse.xmlns !== "urn:dial-multiscreen-org:schemas:dial") {
                              return reject(new Error("xmlns is not 'urn:dial-multiscreen-org:schemas:dial' in the app status response " + parsedResponse.xmlns));
                          }

                          if(body.indexOf("dialVer") !== -1) {
                              parsedResponse.dialVer = body.split("dialVer=")[1].split(" ")[0].split(">")[0].replace(/\s/g, "").replace(/\"/g, "");
                          }
                          if(body.indexOf("allowStop") !== -1) {
                              parsedResponse.allowStop = body.split("<options")[1].split("allowStop=")[1].split(" ")[0].split("/>")[0].replace(/\s/g, "").replace(/\"/g, "");
                          }
                          if(body.indexOf("<link") !== -1) {
                              parsedResponse.rel = body.split("<link rel=")[1].split(" ")[0].split("/>")[0].replace(/\"/g, "");
                              if(parsedResponse.rel !== "run") {
                                  return reject(new Error("@rel attribute is not 'run' in the application status response"));
                              }
                              parsedResponse.href = body.split("href=")[1].split(" ")[0].split("/>")[0].replace(/\s/g, "").replace(/\"/g, "");
                          }
                          if(body.indexOf("<additionalData>") !== -1) {
                              parsedResponse.additionalData = body.split("<additionalData>")[1].split("</additionalData>")[0].replace(/\s/g, "");
                          }
                          return resolve(parsedResponse);
                      }
                      catch (err) {
                          return reject(new Error("There was a problem extracting one or more fields from application status. Status returned : \n" + body));
                      }
                  }
                  return reject(new Error("Error retrieving application status " + error));
              });
          });
      });
}

function launchApplication(host, app, payload) {
    return new Q()
      .then(constructAppResourceUrl.bind(null, host, app))
      // TODO: Send friendlyName query parameter for DIAL 2.1 and greater versions of server
      .then(function (appResourceUrl) {
          var request = {
              url: appResourceUrl,
              method: "POST",
              timeout: 6000,
              headers: {
                  "Content-Type" : "text/plain;charset=\"utf-8\""
              }
          };
          if(!payload) {
              // Set Content-Length: 0
              request.headers["Content-Length"] = 0;
          }
          else {
              // Set the payload
              request.body = payload;
          }
          return new Q.Promise(function (resolve, reject) {
              return httpRequest(request, function handleResponse(error, response) {
                  if(!error) {
                      return resolve(response);
                  }
                  return reject(new Error("Error launching application " + error));
              });
          });
      });
}

function stopApplication(host, app) {
    return new Q()
      .then(constructAppResourceUrl.bind(null, host, app))
      .then(function (appResourceUrl) {
          return getApplicationStatus(host, app)
            .then(function (response) {
                if(response.href) {
                    return appResourceUrl + "/" + response.href; // Construct Application Instance Url
                }
                return Q.reject(new Error("Could not get attribute @href from application status to construct Application Instance Url. "
                      + "This means the DIAL server does not support STOP requests for this application."));
            });
      })
      .then(function (appResourceUrl) {
          var request = {
              url: appResourceUrl,
              method: "DELETE",
              timeout: 6000
          };
          return new Q.Promise(function (resolve, reject) {
              return httpRequest(request, function handleResponse(error, response) {
                  if(!error) {
                      return resolve(response);
                  }
                  return reject(new Error("Error stopping application " + error));
              });
          });
      });
}

function hideApplication(host, app) {
    return new Q()
      .then(constructAppResourceUrl.bind(null, host, app))
      .then(function (appResourceUrl) {
          return getApplicationStatus(host, app)
            .then(function (response) {
                if(response.href) {
                    return appResourceUrl + "/" + response.href + "/hide"; // Construct Application Instance Url
                }
                return Q.reject(new Error("Could not get instance href from application status to construct Application Instance Url"));
            });
      })
      .then(function (appResourceUrl) {
          var request = {
              url: appResourceUrl,
              method: "POST",
              timeout: 6000
          };
          return new Q.Promise(function (resolve, reject) {
              return httpRequest(request, function handleResponse(error, response) {
                  if(!error) {
                      if(response.statusCode === 501) {
                          return reject("HIDE request returned 501 NOT IMPLEMENTED. " +
                              "This means the DIAL server does not support HIDE for this application.");
                      }
                      return resolve(response);
                  }
                  return reject(new Error("Error hiding application " + error));
              });
          });
      });
}

function stopApplicationInstance(instanceUrl) {
    return new Q()
      .then(function () {
          var request = {
              url: instanceUrl,
              method: "DELETE",
              timeout: 6000
          };
          return new Q.Promise(function (resolve, reject) {
              return httpRequest(request, function handleResponse(error, response) {
                  if(!error) {
                      return resolve(response);
                  }
                  return reject(new Error("Error stopping application " + error));
              });
          });
      });
}

function hideApplicationInstance(instanceUrl) {
    return new Q()
      .then(function () {
          var request = {
              url: instanceUrl,
              method: "POST",
              timeout: 6000
          };
          return new Q.Promise(function (resolve, reject) {
              return httpRequest(request, function handleResponse(error, response) {
                  if(!error) {
                      return resolve(response);
                  }
                  return reject(new Error("Error hiding application " + error));
              });
          });
      });
}

function constructAppResourceUrl(host, appName) {
    if(host === undefined) {
        return Q.reject(new Error("Host address is required to construct Application Resource URL"));
    }
    if(appName === undefined) {
        return Q.reject(new Error("Application name is required to construct Application Resource URL"));
    }

    return new Q()
      .then(getAppsUrl.bind(null, host))
      .then(function (appUrl) {
          return appUrl.replace(/\/+$/, "") + "/" + appName;
      });
}

function getAppsUrl(host) {
    if(host === undefined) {
        return Q.reject(new Error("Host is required to query for Application-URL"));
    }
    // Check if value is persisted
    var found = false;
    var appUrl;
    servers.forEach(function (server) {
        if(server.host === host && server.appUrl !== undefined) {
            found = true;
            appUrl = server.appUrl;
        }
    });

    if(found) {
        return appUrl;
    }

    // If value is not persisted
    return new Q()
      .then(getLocation.bind(null, host))
      .then(function (location) {
          return new Q.Promise(function (resolve, reject) {
              return httpRequest({
                  url: location,
                  method: "GET",
                  timeout: 6000
              }, function handleResponse(error, response) {
                  if(!error && response.statusCode === 200) {
                      return resolve(response.headers["application-url"]);
                  }
                  else if(!error) {
                      return reject(new Error("Querying for Application-URL returned status code " + response.statusCode));
                  }
                  return reject(new Error("Querying for Application-URL returned " + error));
              });
          });
      })
      // Persist it
      .then(function (applicationUrl) {
          servers.forEach(function (server) {
              if(server.host === host) {
                  server.appUrl = applicationUrl;
              }
          });
          return applicationUrl;
      });
}

function getLocation(host) {
    if(host === undefined) {
        return Q.reject(new Error("Host address is required to get corresponding LOCATION"));
    }

    var found = false;
    var location;

    // Check if host is present in persisted list
    servers.forEach(function (server) {
        if(server.host === host) {
            location = server.location;
            found = true;
        }
    });
    if(found) {
        return location;
    }

    // If not persisted, perform discovery
    return new Q()
      .then(discoverSpecificHost.bind(null, host))
      .then(function (serverObj) {
          return serverObj.location;
      });
}

function discoverSpecificHost(host) {
    if(host === undefined) {
        return Q.reject(new Error("Host is required field for discoverSpecificHost function"));
    }

    var found = false;
    var serverObjFound;

    servers.forEach(function (server) {
        if(server.host === host) {
            serverObjFound = server;
            found = true;
        }
    });

    if(found) {
        return serverObjFound;
    }
    return new Q.Promise(function (resolve, reject) {
        // On M-Search response
        client.on("response", function (headers) {
            var extractedHost = headers.LOCATION.split("http://")[1].split(":")[0];
            if(host === extractedHost) {
                client.stop();
                var mac, timeout;
                if(headers.WAKEUP) {
                    var wakeup = headers.WAKEUP;
                    mac = wakeup.split(";")[0].split("=")[1];
                    timeout = wakeup.split(";")[1].split("=")[1];
                }
                var serverObj = {
                    location : headers.LOCATION,
                    host : headers.LOCATION.split("http://")[1].split(":")[0],
                    ST : headers.ST,
                    mac : mac,
                    timeout : timeout
                };
                servers.push(serverObj);
                clearTimeout(timer);
                return resolve(serverObj);
            }
        });

        // Send M-Search request
        client.search("urn:dial-multiscreen-org:service:dial:1");

        var timer = setTimeout(function () {
            client.stop();
            return reject(new Error("Could not discover host " + host));
        }, 5000);
    });
}

module.exports.discover                 = discover;
module.exports.discoverSpecificHost     = discoverSpecificHost;
module.exports.getApplicationStatus     = getApplicationStatus;
module.exports.launchApplication        = launchApplication;
module.exports.stopApplication          = stopApplication;
module.exports.hideApplication          = hideApplication;
module.exports.stopApplicationInstance  = stopApplicationInstance;
module.exports.hideApplicationInstance  = hideApplicationInstance;
module.exports.constructAppResourceUrl  = constructAppResourceUrl;
module.exports.getAppsUrl               = getAppsUrl;
module.exports.getLocation              = getLocation;
