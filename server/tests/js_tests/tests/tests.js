"use strict";

var Q = require("q");

var discoverServerUnderTest                      = require("../tests/discoverServerUnderTest.js"),
    launchApplicationNotRecognized               = require("../tests/launchApplicationNotRecognized.js"),
    launchApplicationInRunningStateWithNoPayload = require("../tests/launchApplicationInRunningStateWithNoPayload.js"),
    launchApplicationInRunningStateWithPayload   = require("../tests/launchApplicationInRunningStateWithPayload.js"),
    launchApplicationInStoppedStateWithNoPayload = require("../tests/launchApplicationInStoppedStateWithNoPayload.js"),
    launchApplicationInStoppedStateWithPayload   = require("../tests/launchApplicationInStoppedStateWithPayload.js"),
    launchApplicationInHiddenStateWithNoPayload  = require("../tests/launchApplicationInHiddenStateWithNoPayload.js"),
    launchApplicationInHiddenStateWithPayload    = require("../tests/launchApplicationInHiddenStateWithPayload.js"),
    launchApplicationWithExcessPayload           = require("../tests/launchApplicationWithExcessPayload.js"),
    stopInvalidApplicationInstance               = require("../tests/stopInvalidApplicationInstance.js"),
    stopApplicationInRunningState                = require("../tests/stopApplicationInRunningState.js"),
    stopApplicationInStoppedState                = require("../tests/stopApplicationInStoppedState.js"),
    stopApplicationInHiddenState                 = require("../tests/stopApplicationInHiddenState.js"),
    hideInvalidApplicationInstance               = require("../tests/hideInvalidApplicationInstance.js"),
    hideApplicationInHiddenState                 = require("../tests/hideApplicationInHiddenState.js"),
    hideApplicationInRunningState                = require("../tests/hideApplicationInRunningState.js");

new Q()
  .then(discoverServerUnderTest.test)
  // Application launch tests
  .then(launchApplicationNotRecognized.test)
  .then(launchApplicationInRunningStateWithNoPayload.test)
  .then(launchApplicationInRunningStateWithPayload.test)
  .then(launchApplicationInStoppedStateWithNoPayload.test)
  .then(launchApplicationInStoppedStateWithPayload.test)
  .then(launchApplicationInHiddenStateWithNoPayload.test)
  .then(launchApplicationInHiddenStateWithPayload.test)
  .then(launchApplicationWithExcessPayload.test)

  // Application stop tests
  .then(stopInvalidApplicationInstance.test)
  .then(stopApplicationInRunningState.test)
  .then(stopApplicationInStoppedState.test)
  .then(stopApplicationInHiddenState.test)

  // Application hide tests
  .then(hideInvalidApplicationInstance.test)
  .then(hideApplicationInHiddenState.test)
  .then(hideApplicationInRunningState.test)
  .done();
