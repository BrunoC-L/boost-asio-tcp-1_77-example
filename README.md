# boost asio tcp 1.77 example

Mutliple client sockets connect to a server and execute some data transfers

## Usage

build & run, watch & learn.

## What the hell are those clientDef & serverDef headers?

They are automatically generated headers using [PacketGen](https://github.com/BrunoC-L/PacketGen)

## Should I copy paste this into my production code?

NO. I haven't tried breaking it ***yet***, but if you don't send garbage and don't overflow the currently size 1000 stack allocated char[], there is nearly 0 risk.

The socket error handling is incomplete and PacketGen which this relies on is still WIP. The general idea of this project is to provide a working example of boost asio since many examples are outdated or much too simple.
