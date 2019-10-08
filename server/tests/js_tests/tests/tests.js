"use strict";

var utils = require("../libs/utils.js");

const argv = require("yargs")
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
    .option("timeToWaitForStateChange", {
        alias: "ttw",
        describe: "Time(ms) to wait between state changes before querying application status",
        type: "string",
        default: 5000
    })
    .option("logLevel", {
        alias: "loglevel",
        describe: "The log level for displaying messages to the console, default is `info`', options are info, debug, error, warn",
        type: "string",
        default: "info"
    })
    .help("help").alias("help", "h").argv;

utils.setLogLevel(argv.logLevel);

var discoverServerUnderTest = require("../tests/discoverServerUnderTest.js"),
    launchApplicationNotRecognized = require("../tests/launchApplicationNotRecognized.js"),
    launchApplicationInRunningStateWithNoPayload = require("../tests/launchApplicationInRunningStateWithNoPayload.js"),
    launchApplicationInRunningStateWithPayload = require("../tests/launchApplicationInRunningStateWithPayload.js"),
    launchApplicationInStoppedStateWithNoPayload = require("../tests/launchApplicationInStoppedStateWithNoPayload.js"),
    launchApplicationInStoppedStateWithPayload = require("../tests/launchApplicationInStoppedStateWithPayload.js"),
    launchApplicationInHiddenStateWithNoPayload = require("../tests/launchApplicationInHiddenStateWithNoPayload.js"),
    launchApplicationInHiddenStateWithPayload = require("../tests/launchApplicationInHiddenStateWithPayload.js"),
    launchApplicationWithExcessPayload = require("../tests/launchApplicationWithExcessPayload.js"),
    stopInvalidApplicationInstance = require("../tests/stopInvalidApplicationInstance.js"),
    stopApplicationInRunningState = require("../tests/stopApplicationInRunningState.js"),
    stopApplicationInStoppedState = require("../tests/stopApplicationInStoppedState.js"),
    stopApplicationInHiddenState = require("../tests/stopApplicationInHiddenState.js"),
    hideInvalidApplicationInstance = require("../tests/hideInvalidApplicationInstance.js"),
    hideApplicationInHiddenState = require("../tests/hideApplicationInHiddenState.js"),
    hideApplicationInRunningState = require("../tests/hideApplicationInRunningState.js"),
    sleepTheSystemWithNoKeyWhenItIsRequired = require("../tests/sleepTheSystemWithNoKeyWhenItIsRequired"),
    sleepTheSystemWithKeyWhenItIsRequired = require("../tests/sleepTheSystemWithKeyWhenItIsRequired");

// new Q()
Promise.resolve()
     // System/Sleep tests - these force the system into low power mode
    .then(sleepTheSystemWithNoKeyWhenItIsRequired.test)
    .then(sleepTheSystemWithKeyWhenItIsRequired.test)

    // Discovery Tests
    .then(discoverServerUnderTest.test)

    // Application launch tests
    .then(launchApplicationNotRecognized.test)
    .then(launchApplicationInRunningStateWithNoPayload.test)
    .then(launchApplicationInRunningStateWithPayload.test)
    // .then(launchApplicationInStoppedStateWithNoPayload.test)
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
    .then(hideApplicationInRunningState.test);
