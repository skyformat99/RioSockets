<p align="center"> 
  <img src="https://i.imgur.com/WXE9k2m.png" alt="alt logo">
</p>

[![PayPal](https://github.com/Rageware/Shields/blob/master/paypal.svg)](https://www.paypal.me/nxrighthere) [![Bountysource](https://github.com/Rageware/Shields/blob/master/bountysource.svg)](https://salt.bountysource.com/checkout/amount?team=nxrighthere) [![Coinbase](https://github.com/Rageware/Shields/blob/master/coinbase.svg)](https://commerce.coinbase.com/checkout/03e11816-b6fc-4e14-b974-29a1d0886697)

This is a high-performance, zero-copy, memory-efficient abstraction of UDP sockets over [Registered I/O](https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-server-2012-r2-and-2012/hh997032(v%3Dws.11)) with dual-stack IPv4/IPv6 support. This implementation is based on single-threaded completions polling and utilizes large contiguous page-aligned ring buffers for payloads. It's designed for low-latency and high throughput with large numbers of small messages for applications such as multiplayer games.

Building
--------
The library can be built using [CMake](https://cmake.org/download/) with GNU Make or Visual Studio.

It requires Windows 8 / Windows Server 2012 or higher.

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

    uint8_t* buffer = riosockets_buffer(client, NULL, 1024);

    /*
        Write an outgoing message to `buffer` if it's not `NULL`, multiple buffers can be acquired sequentially
        ...                               
    */

    riosockets_send(client);

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

`RioAddress.ipv6` an IPv6 address.

`RioAddress.ipv4` an IPv4-mapped IPv6 address.

`RioAddress.port` a port number.

### Callbacks
`void (*RioCallback)(RioSocket, const RioAddress*, const uint8_t*, int, RioType)` invoked when a message was received or when send operation was failed with the appropriate data.

### Functions
`riosockets_initialize(void)` initializes the library. Should be called before starting the work. Returns status with a result.

`riosockets_deinitialize(void)` deinitializes the library. Should be called after the work is done.

`riosockets_create(int maxBufferLength, int sendBufferSize, int receiveBufferSize, RioCallback callback, RioError* error)` creates a new socket with a specified size of buffers for sending and receiving. Returns the `RioSocket` handle at success or writes an error. The max buffer length indicates a maximal possible length of a payload per message. The send and receive buffer size indicate the maximal size of ring buffers that sliced for payloads.

`riosockets_destroy(RioSocket* socket)` destroys a socket, frees all allocated memory, and reset the handle.

`riosockets_bind(RioSocket socket, const RioAddress* address)` assigns an address to a socket. The address parameter can be set to `NULL` to let the operating system assign any address. Returns 0 on success or != 0 on failure.

`riosockets_connect(RioSocket socket, const RioAddress* address)` connects a socket to an address. Returns 0 on success or != 0 on failure.

`riosockets_set_option(RioSocket socket, int level, int optionName, const int* optionValue, int optionLength)` sets the current value for a socket option associated with a socket. This function can be used to set platform-specific options that were not specified at socket creation by default. Returns status with a result.

`riosockets_get_option(RioSocket socket, int level, int optionName, int* optionValue, int* optionLength)` gets the current value for a socket option associated with a socket. A length of an option value should be initially set to an appropriate size. Returns status with a result.

`riosockets_buffer(RioSocket socket, const RioAddress* address, int dataLength)` attempts to slice the ring buffer to write a message for a specified address of a receiver. The address parameter can be set to `NULL` if a socket is connected to an address. The data length parameter can't exceed the length that was set at socket creation. If the acquirement of a buffer was failed due to exceeded capacity of the ring buffer, this function will return `NULL`.

`riosockets_send(RioSocket socket)` sends all messages that were written using sliced buffers and checks for completion. This function should be regularly called to ensure that messages are sent to designated receivers. If the sending was failed due to an underlayer error of socket subsystem or kernel interruption, then the callback will be invoked with the appropriate data.

`riosockets_receive(RioSocket socket, int maxCompletions)` receives all messages that were processed by the underlayer socket subsystem after checking for completion. This function should be regularly called to ensure that messages are received from senders. If a message was received successfully, then the callback will be invoked with the appropriate data. The number of completions per call can't exceed the `RIOSOCKETS_MAX_COMPLETION_RESULTS` constant.

`riosockets_address_get(RioSocket socket, RioAddress* address)` gets an address from a bound or connected socket. This function is especially useful to determine the local association that has been set by the operating system. Returns status with a result.

`riosockets_address_is_equal(const RioAddress*, const RioAddress*)` compares two addresses for equality. Returns status with a result.

`riosockets_address_set_ip(RioAddress* address, const char* ip)` sets an IP address. Returns status with a result.

`riosockets_address_get_ip(const RioAddress* address, char* ip, int ipLength)` gets an IP address. The capacity of the string should be equal to `RIOSOCKETS_HOSTNAME_SIZE` constant. Returns status with a result.

`riosockets_address_set_hostname(RioAddress* address, const char* name)` sets host name or an IP address. Returns status with a result.

`riosockets_address_get_hostname(const RioAddress* address, char* name, int nameLength)` attempts to do a reverse lookup from the address. Returns status with a result.
