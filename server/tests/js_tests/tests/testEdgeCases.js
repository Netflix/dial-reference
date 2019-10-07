"use strict";

var dial = require("../libs/dialClient.js"),
    utils = require("../libs/utils.js"),
    Q = require("q"),
    winston = require("winston");


const argv = require("yargs")
    .usage("\nUsage: node " + __filename.slice(__dirname.length + 1) + "[options]")
    .option("host", {
        describe: "IP address of host on which DIAL server under test is running",
        type: "string",
        demand: true
    })
    .help("help").alias("help", "h").argv;

var testCount = 0;
var failureCount = 0;

/*
 *  These edge cases are
 */
function edgeCases() {
    var host = argv.host;

    var methods = [
        "GET",
        "POST",
    ];

    // weird data
    var headers = [
        {"Content-Length": "0xff1af1581"},
        {"Content-Length": "sfsadfdsat"},
        {"Content-Length": 13377777777777},
        {"Content-Length": -13377777777777},
        {"Content-Type": "text/plain;charset=\"utf-8\""},
        {"Content-Type": "text/plain;charset=\"ascii\""},
        {"Content-Type": "asdoffmoaserq"},
        {"Foo": "Bar"},
    ];

    var utf8 = "􎾅򉷰;(QmN􇺥򅟤̸񼙄鱏=叽]𰸹񡻽h˳╭ܮ𻬆煣ِt𐍁J픖pЗ󐰝B-ٴۼ퍃%۞灱Ҡɝ~2񓟟望Lԇƀ􅃣􉭿𯍦ݛ󜫽&±󚀌⫯WܾǊX礴ēʧ񗐦耉ݭpi}Ǖ񨎤ʝ'䬿^ٿ]샑좘쒾"
        + "ԝbւ蹈ۥ웦(歉貤䃶6ƢӋ􃾝􇢼Ø𪺭񃫍񑥽ĩ񳎂k┋譒ց,Ŋ칱آ󃅒3㛣Oܕ=裁f򙗳񢘄Ĵjٕ򳼂pۅ=@ԉ'1󷪁⃮Ȩ핣񸡗𾜃򷞟ƺ񛩨匽ݚ뎔叇χ&죙𱘱ʴ򹏆ذȞ󝗘鞾ﰟO썟X񚗠9򚏗򗆾⛥﫢ȧ走쾙"
        + "򹺡ӱ񗊆إ#!۵Σƀ򽾖4䦼󓋻쪭ϖ0ڧp梛՚嬆߷򽳅򂝍򀡓域᳁١ޜ򵚁ۡ򩺚Ü]ʘ厤ȆwYݖ(ן󛶳􌖂饲a񀪄�`V򵆡<$𚓖ǲ&ܗʧ󋢶􅴹🶝򎆾bҘ@E򥽿1ч󫿘&{򍀓㜃팸󻴡儈Ā㖿rb^Ӟ󗔔ʶ"
        + "􉯨Ө㡷s󢚊Π򍱫򣇭●ŽٗDҤ򄗆Ʊ꫓(򫉦t񴹹-瞜BЏ;Ғ򸻡eࡻs򢺩򅠕`=ݔu樭迕^;𕍃䐈Ǥ3䝡XM붩ݱ􊙩􆰯ϖȓܕ췃񿜵`Ỷ3LV9폧赹ⴢ񁽟+⇩ӛ􂗥뵖띟򦁩䎵钥¶繥Ҕ J礢򼦨м2_8򴻣̑hg";
    utf8 = utf8 + utf8 + utf8;
    var strings = [
        utf8,
        "",
        '<a href="\x19javascript:javascript:alert(1)" id="fuzzelement1">test</a>\n',
        '$HOME',
        '../../../../../../../../../../../etc/hosts\n',
    ];

    // generate test cases
    var testCases = [];
    for (var m = 0; m < methods.length; m++) {
        for (var h = 0; h < headers.length; h++) {
            for (var q = 0; q < strings.length; q++) {
                for (var b = 0; b < strings.length; b++) {
                    testCases.push(function (m, h, q, b) {
                        return new Q()
                            .then(dial.sendRequest.bind(null, host, methods[m], headers[h], strings[q], strings[b]))
                            .then(function (response) {
                                testCount = testCount + 1;
                                if (![400, 404, 500].includes(response.statusCode)) {
                                    return Q.reject(new Error("Sent the DIAL server an edge case. Expected a bad status code but got " + response.statusCode));
                                }
                            })
                            .fail(function handleError(err) {
                                utils.printTestInfo(__filename.slice(__dirname.length + 1),
                                    "Edge Case, method: " + methods[m] + " header: "
                                    + headers[h] + " query string: "
                                    + strings[q] + " body: " + strings[b]);
                                utils.printTestFailure(err);
                                failureCount = failureCount + 1;
                                return err;
                            })
                    }.bind(null, m, h, q, b));
                }
            }
        }
    }

    return testCases;
}

winston.info("Testing edge cases, only failing tests will appear.");
return edgeCases().reduce(Q.when, Promise.resolve()).done(function() {
    winston.info("Tests complete. Passing: " + (testCount - failureCount) + ", Failures: " + failureCount);
});
