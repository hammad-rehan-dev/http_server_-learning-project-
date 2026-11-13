# HTTP Server (learning project)

A minimal HTTP server in C++ using raw POSIX sockets. Serves static files over IPv6 on port 8080.

## Build

```bash
g++ -o obj http_server.cpp
```

## Run

Pass the path to your root HTML file. Other files are served relative to its directory.

```bash
./obj /path/to/web/index.html
```

Then open http://localhost:8080

## Debug

Build with debug symbols and warnings:

```bash
g++ -g -Wall -o obj http_server.cpp
```

Run under gdb:

```bash
gdb --args ./obj /path/to/web/index.html
```

Save the server log (requests, headers sent, errors) to a file:

```bash
./obj /path/to/web/index.html > debug.txt 2>&1
```
