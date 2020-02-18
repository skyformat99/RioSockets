<p align="center"> 
  <img src="https://i.imgur.com/WXE9k2m.png" alt="alt logo">
</p>

[![PayPal](https://github.com/Rageware/Shields/blob/master/paypal.svg)](https://www.paypal.me/nxrighthere) [![Bountysource](https://github.com/Rageware/Shields/blob/master/bountysource.svg)](https://salt.bountysource.com/checkout/amount?team=nxrighthere) [![Coinbase](https://github.com/Rageware/Shields/blob/master/coinbase.svg)](https://commerce.coinbase.com/checkout/03e11816-b6fc-4e14-b974-29a1d0886697)

This is a high-performance, zero-copy, memory-efficient abstraction of UDP sockets over [Registered I/O](https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-server-2012-r2-and-2012/hh997032(v%3Dws.11)) with dual-stack IPv4/IPv6 support. This implementation is based on single-threaded completions polling and utilizes large contiguous page-aligned ring buffers for payloads. It's designed for low-latency and high throughput with large numbers of small messages for applications such as multiplayer games.

Building
--------
The library can be built using [CMake](https://cmake.org/download/) with GNU Make or Visual Studio.

It requires Windows 8 \ Windows Server 2012 or higher.

Usage
--------
Before starting to work, the library should be initialized using `riosockets_initialize();` function.

After the work is done, deinitialize the library using `riosockets_deinitialize();` function.

##### Start a new server:
```c
void callback(RioSocket server, const RioAddress* address, const uint8_t* data, int dataLength, RioType type) {
    if (type == RIOSOCKETS_TYPE_RECEIVE) {
        char ip[RIOSOCKETS_HOSTNAME_SIZE] = { 0 };

        riosockets_address_get_ip(address, ip, sizeof(ip));

        printf("Message received from - IP: %s, Data length: %i\n", ip, dataLength);

        /*
            Read an incoming message from `data`
            ...                               
        */
    } else {
        printf("Message sending was failed!\n");
    }
}

RioError error = RIOSOCKETS_ERROR_NONE;
RioSocket server = riosockets_create(1024, 256 * 1024, 256 * 1024, callback, &error);

if (error != RIOSOCKETS_ERROR_NONE) {
    printf("Socket creation failed! Error code: %i\n", error);
} else {
    RioAddress listenAddress = { 0 };

    listenAddress.port = port;

    if (riosockets_address_set_ip(&listenAddress, "::0") == RIOSOCKETS_STATUS_OK)
        printf("Address set!\n");

    if (riosockets_bind(server, &listenAddress) == 0)
        printf("Socket bound!\n");

    char ip[RIOSOCKETS_HOSTNAME_SIZE] = { 0 };
    RioAddress address = { 0 };

    while (!_kbhit()) {
        riosockets_receive(server, RIOSOCKETS_MAX_COMPLETION_RESULTS);
    }

    riosockets_destroy(&server);
}
```
##### Start a new client:
```c
void callback(RioSocket client, const RioAddress* address, const uint8_t* data, int dataLength, RioType type) {
    if (type == RIOSOCKETS_TYPE_RECEIVE) {
        printf("Message received from server - Data length: %i\n", dataLength);

        /*
            Read an incoming message from `data`
            ...                               
        */
    } else {
        printf("Message sending was failed!\n");
    }
}

RioError error = RIOSOCKETS_ERROR_NONE;
RioSocket client = riosockets_create(1024, 256 * 1024, 256 * 1024, callback, &error);

if (error != RIOSOCKETS_ERROR_NONE) {
    printf("Socket creation failed! Error code: %i\n", error);
} else {
    RioAddress connectionAddress = { 0 };

    connectionAddress.port = port;

    if (riosockets_address_set_ip(&connectionAddress, "::1") == RIOSOCKETS_STATUS_OK)
        printf("Address set!\n");

    if (riosockets_connect(client, &connectionAddress) == 0)
        printf("Socket connected!\n");

    uint8_t* buffer = riosockets_buffer(client, address, 1024);

    /*
        Write an outgoing message to `buffer` of it's not `NULL`
        ...                               
    */

    riosockets_send(server);

    while (!_kbhit()) {
        riosockets_receive(client, RIOSOCKETS_MAX_COMPLETION_RESULTS);
    }

    riosockets_destroy(&client);
}
```

API reference
--------
### Type definitions
#### RioSocket
An integer type with the socket handle.

### Enumerations
#### RioStatus
Definitions of status types for functions:

`RIOSOCKETS_STATUS_OK`

`RIOSOCKETS_STATUS_ERROR`

#### RioType
Definitions of event types for registered callback:

`RIOSOCKETS_TYPE_SEND`

`RIOSOCKETS_TYPE_RECEIVE`

#### RioError
Definitions of error types for the socket creation function:

`RIOSOCKETS_ERROR_NONE`

`RIOSOCKETS_ERROR_SOCKET_CREATION`

`RIOSOCKETS_ERROR_SOCKET_DUAL_STACK`

`RIOSOCKETS_ERROR_RIO_EXTENSION`

`RIOSOCKETS_ERROR_RIO_BUFFER_SIZE`

`RIOSOCKETS_ERROR_RIO_EVENT`

`RIOSOCKETS_ERROR_RIO_COMPLETION_QUEUE`

`RIOSOCKETS_ERROR_RIO_REQUEST_QUEUE`

`RIOSOCKETS_ERROR_RIO_BUFFER_CREATION`

`RIOSOCKETS_ERROR_RIO_BUFFER_REGISTRATION`

`RIOSOCKETS_ERROR_RIO_BUFFER_ASSOCIATION`

### Structures
#### RioAddress
Contains a structure with host data and port number.

`RioAddress.ipv6`

`RioAddress.ipv4`

`RioAddress.port`

### Callbacks
`void (*RioCallback)(RioSocket, const RioAddress*, const uint8_t*, int, RioType)`

### Functions
`riosockets_initialize(void)`

`riosockets_deinitialize(void)`

`riosockets_create(int, int, int, RioCallback, RioError*)`

`riosockets_destroy(RioSocket*)`

`riosockets_bind(RioSocket, const RioAddress*)`

`riosockets_connect(RioSocket, const RioAddress*)`

`riosockets_set_option(RioSocket, int, int, const int*, int)`

`riosockets_get_option(RioSocket, int, int, int*, int*)`

`riosockets_buffer(RioSocket, const RioAddress*, int)`

`riosockets_send(RioSocket)`

`riosockets_receive(RioSocket, int)`

`riosockets_address_is_equal(const RioAddress*, const RioAddress*)`

`riosockets_address_set_ip(RioAddress*, const char*)`

`riosockets_address_get_ip(const RioAddress*, char*, int)`

`riosockets_address_set_hostname(RioAddress*, const char*)`

`riosockets_address_get_hostname(const RioAddress*, char*, int)`
