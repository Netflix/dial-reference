# DIAL 


<div align="center">
  <img src="https://d2640ba2-a-c2dc72bd-s-sites.googlegroups.com/a/dial-multiscreen.org/dial/home/dial-icon.png?attachauth=ANoY7cr5UqoJK0l_igjP6UR00xVd51_0WW3flff7HsLb5tsJfASpJJeNY5AMTs_5CYqqspoLOzYTy8ep8_aXzmLFps9-UIFpkepxOwv5m2JeIWl3Rf-9KSDUI12nYakfHmQY9PThoIqJNwom9BO3DWDX_lhDHU55JWJS427P5UjpoiR3N2p02vnpR5ABHNBpTGfQnKkHvZHpXskH6J5hJ6LRg1rn2Jx0SA%3D%3D&attredirects=0"><br><br>
</div>

**DI**scovery **A**nd **L**aunch—is a simple protocol that second-screen devices can use to discover and launch apps on first-screen devices. 

[DIAL page](http://www.dial-multiscreen.org/) |
[DIAL protocol spec](http://www.dial-multiscreen.org/dial-protocol-specification)



## Building the DIAL server
1) Define the TARGET environment variable to point to the CC compiler prefix
   for your target platform.


2) Run make, passing in your TARGET value.
```
   For example:
   TARGET=/usr/local/i686-DIAL-EXAMPLE/bin/i686-DIAL-EXAMPLE make
```
### Running the DIAL server
The DIAL server should be started as a service, after the platform's networking
has been initialized, and it should remain running at all times (a daemon
process in the system).


## Building the DIAL client
The DIAL client is a standalone C++ console application you can use to test
a running DIAL server implementation on your device. Unlike the server, which
is built for, and meant to run on your device, the client is meant to run on
your desktop (development) machine.

The DIAL client uses CURL to send HTTP REST commands to the DIAL server, so to
build the client, you need to ensure that the CURL dependencies are
defined properly.

Alternatively, you can build against a different, current version of libcurl.
Adjust the INCLUDES and LDFLAGS definitions to point to your actual libcurl
header and library locations. In most cases, you can omit the TARGET define.

Note: the -rpath argument passed to LDFLAGS specifies the libcurl location
to the runtime linker.

### Running the DIAL client in interactive (menu) mode
1) The DIAL client application must be running in the same subnet as the
   DIAL server.

2) Start the client: ./dialclient (or ./dialclient -m)
   The on-screen menu will list all available actions.

### Running the DIAL client in conformance test (non-interactive) mode
1) The DIAL client application must be running in the same subnet as the
   DIAL server.

2) Start the client:
   ./dialclient -i [input-file] [-o output-file] [-a server-IP-addr]

   In script-driven mode, the client reads in an input-file, executes the
   instructions in the input-file, and generates a
   report. The default file locations (which can be overridden) are:
      ./dialclient_input.txt
      ./report.html

## DIAL client Usage
When running the DIAL client, you have the following options
```
usage: dialclient <option>

Option Parameter          Description
 -h     none               Usage menu
 -m     none               Use menu
 -o     filename           Reporter output file (./report.html)
 -i     filename           Input File (./dialclient_input.txt)
 -a     ip_address         IP addr of DIAL server (used for conformance testing)
```
If you do not provide an ip_address and multiple servers are discovered, the
client will prompt you to select a server.

## NEW: Node.js tests for DIAL server 2.1
Node.js tests to test DIAL server 2.1 implementation are now available under
server/tests/js_tests. To run these tests againsts a DIAL server:

1. Ensure that the DIAL server is discoverable from the test environment
2. Install node in the test environment
3. From the directory where the package.json is located (server/tests/js_tests),
      npm install

The tests themselves are located inside the server/tests/js_tests/tests folder.
The file tests.js is a batch runner and will run all the tests serially. It
takes the following arguments:
```
server/tests/js_tests/tests$ node tests.js

Usage: node tests.js[options]

Options:
  --host                             IP address of host on which DIAL server
                                     under test is running   [string] [required]
  --application, --app               Application to test     [string] [required]
  --timeToWaitForStateChange, --ttw  Time(ms) to wait between state changes
                                     before querying application status
                                                        [string] [default: 5000]
  --help, -h                         Show help                         [boolean]
```

To run each test independently and not through tests.js, just call the
appropriate test file name.
```
Example:
server/tests/js_tests/tests$ node discoverServerUnderTest.js

Usage: node discoverServerUnderTest.js[options]

Options:
  --host                IP address of host on which DIAL server under test is
                        running                              [string] [required]
  --application, --app  Application to test                  [string] [required]
  --help, -h            Show help                                      [boolean]
```
Log file of test run is written in js_tests_log.txt in the
server/tests/js_tests/tests folder.

This reference does not provide code for sleeping an app, so
tests that involve sleeping an app will fail.

There is also a test suite for lightly testing how the implementation handles edge cases.
this can be run with:
```
server/tests/js_tests/tests$ node testEdgeCases.js 

Usage: node testEdgeCases.js[options]

Options:
  --host      IP address of host on which DIAL server under test is running
                                                             [string] [required]
  --help, -h  Show help                                                [boolean]
```
