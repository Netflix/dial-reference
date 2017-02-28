"use strict";

var Q = require("q");

var discoverServerUnderTest                  = require("../tests/discoverServerUnderTest.js"),
    launchInvalidApp                         = require("../tests/launchAppNotRecognized.js"),
    launchNetflixInRunningStateWithNoPayload = require("../tests/launchNetflixAlreadyRunningWithNoPayload.js"),
    launchNetflixInRunningStateWithPayload   = require("../tests/launchNetflixAlreadyRunningWithPayload.js"),
    launchNetflixInStoppedStateWithNoPayload = require("../tests/launchNetflixNotRunningWithNoPayload.js"),
    launchNetflixInStoppedStateWithPayload   = require("../tests/launchNetflixNotRunningWithPayload.js"),
    launchNetflixWhenHiddenWithNoPayload     = require("../tests/launchNetflixWhenHiddenWithNoPayload.js"),
    launchNetflixWhenHiddenWithPayload       = require("../tests/launchNetflixWhenHiddenWithPayload.js"),
    launchNetflixWithExcessPayload           = require("../tests/launchNetflixWithExcessPayload.js"),
    stopInvalidNetflixInstance               = require("../tests/stopInvalidNetflixInstance.js"),
    stopNetflixInRunningState                = require("../tests/stopNetflixInRunningState.js"),
    stopNetflixInStoppedState                = require("../tests/stopNetflixInStoppedState.js"),
    stopNetflixInHiddenState                 = require("../tests/stopNetflixInHiddenState.js"),
    hideInvalidApplicationInstance           = require("../tests/hideInvalidApplicationInstance.js"),
    hideNetflixWhenAlreadyHidden             = require("../tests/hideNetflixWhenAlreadyHidden.js"),
    hideNetflixWhenRunning                   = require("../tests/hideNetflixWhenRunning.js");

new Q()
  .then(discoverServerUnderTest.test)
  .then(launchInvalidApp.test)
  .then(launchNetflixInRunningStateWithNoPayload.test)
  .then(launchNetflixInRunningStateWithPayload.test)
  .then(launchNetflixInStoppedStateWithNoPayload.test)
  .then(launchNetflixInStoppedStateWithPayload.test)
  .then(launchNetflixWhenHiddenWithNoPayload.test)
  .then(launchNetflixWhenHiddenWithPayload.test)
  .then(launchNetflixWithExcessPayload.test)
  .then(stopInvalidNetflixInstance.test)
  .then(stopNetflixInRunningState.test)
  .then(stopNetflixInStoppedState.test)
  .then(stopNetflixInHiddenState.test)
  .then(hideInvalidApplicationInstance.test)
  .then(hideNetflixWhenAlreadyHidden.test)
  .then(hideNetflixWhenRunning.test)
  .done();
