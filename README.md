<p align="center"> 
  <img src="https://i.imgur.com/WXE9k2m.png" alt="alt logo">
</p>

[![PayPal](https://github.com/nxrighthere/Shields/blob/master/paypal.svg)](https://www.paypal.me/nxrighthere) [![Bountysource](https://github.com/nxrighthere/Shields/blob/master/bountysource.svg)](https://salt.bountysource.com/checkout/amount?team=nxrighthere) [![Coinbase](https://github.com/nxrighthere/Shields/blob/master/coinbase.svg)](https://commerce.coinbase.com/checkout/03e11816-b6fc-4e14-b974-29a1d0886697)

This is a high-performance, zero-copy, memory-efficient abstraction of UDP sockets over [Registered I/O](https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-server-2012-r2-and-2012/hh997032(v%3Dws.11)) with dual-stack IPv4/IPv6 support. This implementation is based on single-threaded completions polling and utilizes large contiguous page-aligned ring buffers for payloads. It's designed for low-latency and high throughput with large numbers of small messages for applications such as multiplayer games.

Building
--------
The library can be built using [CMake](https://cmake.org/download/) with GNU Make or Visual Studio.

It requires Windows 8 \ Windows Server 2012 or higher.

Usage
--------


API reference
--------

