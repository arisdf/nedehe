Nedehe
======================================================================

Nedehe - Networking Debug Help programs: Nedehe is a set of c++ programs to help debug my networking programming of boost::asio and boost::beast.

* boost::asio
* boost::beast

ws_client
-----------------------------------

Example HowTo:

```
ws_client 127.0.0.1 9001 wr
```

Connect to a websocket server hosted on 127.0.0.1 9001, then chat with the server.

Mode:

* wr - Write then read
* rw - Read then write
* sim - Read and write simultaneously (two separated threads)

websocket client - help debug c++ websocket server programming.

Build
-----------------------------------

Install b2 build,

https://bfgroup.xyz

then build nedehe:

```
cd nedehe
b2
```


LICENSE
-----------------------------------

Boost Software License

COPYRIGHT
-----------------------------------

arisdf.i2p

