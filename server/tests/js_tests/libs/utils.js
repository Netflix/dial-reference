"use strict";

function getParam(key) {
    var value;
    var args = process.argv.slice(2);
    args.forEach(function (param) {
        if(param.indexOf(key + "=") === 0) {
            value = param.split("=")[1];
        }
    });
    return value;
}

module.exports.getParam = getParam;
