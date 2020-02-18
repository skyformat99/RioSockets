<p align="center"> 
  <img src="https://i.imgur.com/WXE9k2m.png" alt="alt logo">
</p>

[![PayPal](https://drive.google.com/uc?id=1OQrtNBVJehNVxgPf6T6yX1wIysz1ElLR)](https://www.paypal.me/nxrighthere) [![Bountysource](https://drive.google.com/uc?id=19QRobscL8Ir2RL489IbVjcw3fULfWS_Q)](https://salt.bountysource.com/checkout/amount?team=nxrighthere) [![Coinbase](https://drive.google.com/uc?id=1LckuF-IAod6xmO9yF-jhTjq1m-4f7cgF)](https://commerce.coinbase.com/checkout/03e11816-b6fc-4e14-b974-29a1d0886697)

This is a high-performance, zero-copy, memory-efficient abstraction of UDP sockets over [Registered I/O](https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-server-2012-r2-and-2012/hh997032(v%3Dws.11)) with dual-stack IPv4/IPv6 support. This implementation is based on single-threaded completions polling and utilizes large contiguous page-aligned ring buffers for payloads. It's designed for low-latency and high throughput with large numbers of small messages for applications such as multiplayer games.

Building
--------
The library can be built using [CMake](https://cmake.org/download/) with GNU Make or Visual Studio.

It requires Windows 8 \ Windows Server 2012 or higher.

Usage
--------


API reference
--------

