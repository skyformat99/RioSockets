#ifndef RIOSOCKETS_H
#define RIOSOCKETS_H

#include <stdint.h>
#include <ws2tcpip.h>

#define RIOSOCKETS_VERSION_MAJOR 1
#define RIOSOCKETS_VERSION_MINOR 0
#define RIOSOCKETS_VERSION_PATCH 0

#define RIOSOCKETS_CALLBACK __cdecl

#ifdef RIOSOCKETS_DLL
	#ifdef RIOSOCKETS_IMPLEMENTATION
		#define RIOSOCKETS_API __declspec(dllexport)
	#else
		#define RIOSOCKETS_API __declspec(dllimport)
	#endif
#endif

#define RIOSOCKETS_HOSTNAME_SIZE 1025
#define RIOSOCKETS_MAX_COMPLETION_RESULTS 256

// API

#ifdef __cplusplus
extern "C" {
#endif

	typedef intptr_t RioSocket;

	typedef enum _RioStatus {
		RIOSOCKETS_STATUS_OK = 0,
		RIOSOCKETS_STATUS_ERROR = -1
	} RioStatus;

	typedef enum _RioType {
		RIOSOCKETS_TYPE_SEND = 0,
		RIOSOCKETS_TYPE_RECEIVE = 1
	} RioType;

	typedef enum _RioError {
		RIOSOCKETS_ERROR_NONE = 0,
		RIOSOCKETS_ERROR_SOCKET_CREATION = 1,
		RIOSOCKETS_ERROR_SOCKET_DUAL_STACK = 2,
		RIOSOCKETS_ERROR_RIO_EXTENSION = 3,
		RIOSOCKETS_ERROR_RIO_BUFFER_SIZE = 4,
		RIOSOCKETS_ERROR_RIO_EVENT = 5,
		RIOSOCKETS_ERROR_RIO_COMPLETION_QUEUE = 6,
		RIOSOCKETS_ERROR_RIO_REQUEST_QUEUE = 7,
		RIOSOCKETS_ERROR_RIO_BUFFER_CREATION = 8,
		RIOSOCKETS_ERROR_RIO_BUFFER_REGISTRATION = 9,
		RIOSOCKETS_ERROR_RIO_BUFFER_ASSOCIATION = 10
	} RioError;

	typedef struct _RioAddress {
		union {
			struct in6_addr ipv6;
			struct {
				uint8_t zeros[10];
				uint16_t ffff;
				struct in_addr ip;
			} ipv4;
		};
		uint16_t port;
	} RioAddress;

	typedef void (RIOSOCKETS_CALLBACK *RioCallback)(RioSocket, const RioAddress*, const uint8_t*, int, RioType);

	RIOSOCKETS_API RioStatus riosockets_initialize(void);

	RIOSOCKETS_API void riosockets_deinitialize(void);

	RIOSOCKETS_API RioSocket riosockets_create(int, int, int, RioCallback, RioError*);

	RIOSOCKETS_API void riosockets_destroy(RioSocket*);

	RIOSOCKETS_API int riosockets_bind(RioSocket, const RioAddress*);

	RIOSOCKETS_API int riosockets_connect(RioSocket, const RioAddress*);

	RIOSOCKETS_API RioStatus riosockets_set_option(RioSocket, int, int, const int*, int);

	RIOSOCKETS_API RioStatus riosockets_get_option(RioSocket, int, int, int*, int*);

	RIOSOCKETS_API uint8_t* riosockets_buffer(RioSocket, const RioAddress*, int);

	RIOSOCKETS_API void riosockets_send(RioSocket);

	RIOSOCKETS_API void riosockets_receive(RioSocket, int);

	RIOSOCKETS_API RioStatus riosockets_address_is_equal(const RioAddress*, const RioAddress*);

	RIOSOCKETS_API RioStatus riosockets_address_set_ip(RioAddress*, const char*);

	RIOSOCKETS_API RioStatus riosockets_address_get_ip(const RioAddress*, char*, int);

	RIOSOCKETS_API RioStatus riosockets_address_set_hostname(RioAddress*, const char*);

	RIOSOCKETS_API RioStatus riosockets_address_get_hostname(const RioAddress*, char*, int);

#ifdef __cplusplus
}
#endif

#if defined(RIOSOCKETS_IMPLEMENTATION) && !defined(RIOSOCKETS_IMPLEMENTATION_DONE)
	#define RIOSOCKETS_IMPLEMENTATION_DONE 1

	#include <string.h>
	#include <versionhelpers.h>
	#include <mswsock.h>

	#ifdef __MINGW32__
		#include "mingw/rio.h"
	#endif

	typedef struct _RioBuffer {
		RIO_BUF data;
		RIO_BUF address;
		BOOL addressless;
	} RioBuffer;

	typedef struct _Rio {
		RIO_EXTENSION_FUNCTION_TABLE functions;
		RIO_CQ sendQueue;
		RIO_CQ receiveQueue;
		RIO_RQ requestQueue;
		WSAEVENT sendEvent;
		WSAEVENT receiveEvent;
		SOCKET socket;
		char* sendMemory;
		char* sendMemoryAddress;
		char* receiveMemory;
		char* receiveMemoryAddress;
		RioBuffer* sendBuffers;
		RioBuffer* receiveBuffers;
		RioCallback callback;
		int maxBufferLength;
		int sendBufferCount;
		int sendBufferQueue;
		int sendBufferTail;
		int sendBufferPending;
		int receiveBufferCount;
		int receiveBufferHead;
	} Rio;

	// Macros

	#define RIOSOCKETS_HOST_TO_NET_16(value) (htons(value))
	#define RIOSOCKETS_HOST_TO_NET_32(value) (htonl(value))
	#define RIOSOCKETS_NET_TO_HOST_16(value) (ntohs(value))
	#define RIOSOCKETS_NET_TO_HOST_32(value) (ntohl(value))

	// Functions

	inline static uint64_t riosockets_round_and_divide(uint64_t value, uint64_t roundTo) {
		return ((value + roundTo - 1) / roundTo);
	}

	inline static uint64_t riosockets_round_up(uint64_t value, uint64_t roundTo) {
		return riosockets_round_and_divide(value, roundTo) * roundTo;
	}

	inline static int riosockets_array_is_zeroed(const uint8_t* array, int length) {
		for (size_t i = 0; i < length; i++) {
			if (array[i] != 0)
				return -1;
		}

		return 0;
	}

	inline static size_t riosockets_string_copy(char* destination, const char* source, size_t length) {
		char* d = destination;
		const char* s = source;
		size_t n = length;

		if (n != 0 && --n != 0) {
			do {
				if ((*d++ = *s++) == 0)
					break;
			}

			while (--n != 0);
		}

		if (n == 0) {
			if (length != 0)
				*d = '\0';

			while (*s++);
		}

		return (s - source - 1);
	}

	inline static void riosockets_address_extract(RioAddress* address, const struct sockaddr_storage* source) {
		if (source->ss_family == AF_INET) {
			struct sockaddr_in* socketAddress = (struct sockaddr_in*)source;

			memset(address, 0, sizeof(address->ipv4.zeros));

			address->ipv4.ffff = 0xFFFF;
			address->ipv4.ip = socketAddress->sin_addr;
			address->port = RIOSOCKETS_NET_TO_HOST_16(socketAddress->sin_port);
		} else if (source->ss_family == AF_INET6) {
			struct sockaddr_in6* socketAddress = (struct sockaddr_in6*)source;

			address->ipv6 = socketAddress->sin6_addr;
			address->port = RIOSOCKETS_NET_TO_HOST_16(socketAddress->sin6_port);
		}
	}

	static char* riosockets_buffer_allocate(uint64_t bufferLength, uint64_t bufferCount) {
		SYSTEM_INFO systemInfo = { 0 };

		GetSystemInfo(&systemInfo);

		return VirtualAlloc(NULL, riosockets_round_up(bufferLength * bufferCount, systemInfo.dwPageSize), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	}

	static void riosockets_buffer_free(char* buffer) {
		if (buffer != NULL)
			VirtualFree(buffer, 0, MEM_RELEASE);
	}

	RioStatus riosockets_initialize(void) {
		WSADATA wsaData = { 0 };

		if (WSAStartup(MAKEWORD(2, 2), &wsaData))
			return RIOSOCKETS_STATUS_ERROR;

		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || !IsWindows8OrGreater()) {
			WSACleanup();

			return RIOSOCKETS_STATUS_ERROR;
		}

		return RIOSOCKETS_STATUS_OK;
	}

	void riosockets_deinitialize(void) {
		WSACleanup();
	}

	RioSocket riosockets_create(int maxBufferLength, int sendBufferSize, int receiveBufferSize, RioCallback callback, RioError* error) {
		Rio* rio = NULL;

		if (callback == NULL || error == NULL)
			return -1;

		SOCKET socket = WSASocketW(PF_INET6, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_REGISTERED_IO);

		if (socket != INVALID_SOCKET) {
			int onlyIPv6 = 0;

			if (setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&onlyIPv6, sizeof(onlyIPv6)) != 0) {
				closesocket(socket);

				*error = RIOSOCKETS_ERROR_SOCKET_DUAL_STACK;

				return -1;
			}

			rio = (Rio*)calloc(1, sizeof(Rio));

			rio->socket = socket;
			rio->maxBufferLength = maxBufferLength;
			rio->callback = callback;

			GUID functionTableID = WSAID_MULTIPLE_RIO;
			DWORD outBytes = 0;

			if (WSAIoctl(socket, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &functionTableID, sizeof(functionTableID), (void**)&rio->functions, sizeof(rio->functions), &outBytes, 0, 0) != 0) {
				*error = RIOSOCKETS_ERROR_RIO_EXTENSION;

				goto destroy;
			}

			if (sendBufferSize < rio->maxBufferLength || receiveBufferSize < rio->maxBufferLength) {
				*error = RIOSOCKETS_ERROR_RIO_BUFFER_SIZE;

				goto destroy;
			}

			rio->sendBufferCount = sendBufferSize / rio->maxBufferLength;
			rio->receiveBufferCount = receiveBufferSize / rio->maxBufferLength;

			RIO_NOTIFICATION_COMPLETION sendQueue = { 0 };

			rio->sendEvent = WSACreateEvent();

			if (rio->sendEvent == WSA_INVALID_EVENT) {
				*error = RIOSOCKETS_ERROR_RIO_EVENT;

				goto destroy;
			}

			sendQueue.Type = RIO_EVENT_COMPLETION;
			sendQueue.Event.EventHandle = rio->sendEvent;

			rio->sendQueue = rio->functions.RIOCreateCompletionQueue(rio->sendBufferCount, &sendQueue);

			RIO_NOTIFICATION_COMPLETION receiveQueue = { 0 };

			rio->receiveEvent = WSACreateEvent();

			if (rio->receiveEvent == WSA_INVALID_EVENT) {
				*error = RIOSOCKETS_ERROR_RIO_EVENT;

				goto destroy;
			}

			receiveQueue.Type = RIO_EVENT_COMPLETION;
			receiveQueue.Event.EventHandle = rio->receiveEvent;

			rio->receiveQueue = rio->functions.RIOCreateCompletionQueue(rio->receiveBufferCount, &receiveQueue);

			if (rio->sendQueue == RIO_INVALID_CQ || rio->receiveQueue == RIO_INVALID_CQ) {
				*error = RIOSOCKETS_ERROR_RIO_COMPLETION_QUEUE;

				goto destroy;
			}

			rio->requestQueue = rio->functions.RIOCreateRequestQueue(socket, rio->receiveBufferCount, 1, rio->sendBufferCount, 1, rio->receiveQueue, rio->sendQueue, 0);

			if (rio->requestQueue == RIO_INVALID_RQ) {
				*error = RIOSOCKETS_ERROR_RIO_REQUEST_QUEUE;

				goto destroy;
			}

			rio->sendMemory = riosockets_buffer_allocate(rio->maxBufferLength, rio->sendBufferCount);
			rio->sendMemoryAddress = riosockets_buffer_allocate(sizeof(SOCKADDR_INET), rio->sendBufferCount);

			if (rio->sendMemory == NULL || rio->sendMemoryAddress == NULL) {
				*error = RIOSOCKETS_ERROR_RIO_BUFFER_CREATION;

				goto destroy;
			}

			RIO_BUFFERID sendBufferID = rio->functions.RIORegisterBuffer(rio->sendMemory, rio->maxBufferLength * rio->sendBufferCount);
			RIO_BUFFERID sendAddressBufferID = rio->functions.RIORegisterBuffer(rio->sendMemoryAddress, rio->sendBufferCount * sizeof(SOCKADDR_INET));

			if (sendBufferID == RIO_INVALID_BUFFERID || sendAddressBufferID == RIO_INVALID_BUFFERID) {
				*error = RIOSOCKETS_ERROR_RIO_BUFFER_REGISTRATION;

				goto destroy;
			}

			rio->sendBuffers = (RioBuffer*)calloc(rio->sendBufferCount, sizeof(RioBuffer));

			for (int i = 0; i < rio->sendBufferCount; ++i) {
				RIO_BUF buffer = { 0 };

				buffer.BufferId = sendBufferID;
				buffer.Offset = rio->maxBufferLength * i;
				buffer.Length = rio->maxBufferLength;

				rio->sendBuffers[i].data = buffer;

				buffer.BufferId = sendAddressBufferID;
				buffer.Offset = sizeof(SOCKADDR_INET) * i;
				buffer.Length = sizeof(SOCKADDR_INET);

				rio->sendBuffers[i].address = buffer;
			}

			rio->receiveMemory = riosockets_buffer_allocate(rio->maxBufferLength, rio->receiveBufferCount);
			rio->receiveMemoryAddress = riosockets_buffer_allocate(sizeof(SOCKADDR_INET), rio->receiveBufferCount);

			if (rio->receiveMemory == NULL || rio->receiveMemoryAddress == NULL) {
				*error = RIOSOCKETS_ERROR_RIO_BUFFER_CREATION;

				goto destroy;
			}

			RIO_BUFFERID receiveBufferID = rio->functions.RIORegisterBuffer(rio->receiveMemory, rio->maxBufferLength * rio->receiveBufferCount);
			RIO_BUFFERID receiveAddressBufferID = rio->functions.RIORegisterBuffer(rio->receiveMemoryAddress, rio->receiveBufferCount * sizeof(SOCKADDR_INET));

			if (receiveBufferID == RIO_INVALID_BUFFERID || receiveAddressBufferID == RIO_INVALID_BUFFERID) {
				*error = RIOSOCKETS_ERROR_RIO_BUFFER_REGISTRATION;

				goto destroy;
			}

			rio->receiveBuffers = (RioBuffer*)calloc(rio->receiveBufferCount, sizeof(RioBuffer));

			for (int i = 0; i < rio->receiveBufferCount; ++i) {
				RIO_BUF buffer = { 0 };

				buffer.BufferId = receiveBufferID;
				buffer.Offset = rio->maxBufferLength * i;
				buffer.Length = rio->maxBufferLength;

				rio->receiveBuffers[i].data = buffer;

				buffer.BufferId = receiveAddressBufferID;
				buffer.Offset = sizeof(SOCKADDR_INET) * i;
				buffer.Length = sizeof(SOCKADDR_INET);

				rio->receiveBuffers[i].address = buffer;

				if (!rio->functions.RIOReceiveEx(rio->requestQueue, &rio->receiveBuffers[i].data, 1, NULL, &rio->receiveBuffers[i].address, NULL, NULL, 0, 0)) {
					*error = RIOSOCKETS_ERROR_RIO_BUFFER_ASSOCIATION;

					goto destroy;
				}
			}

			goto create;

			destroy:

			riosockets_destroy((RioSocket*)&rio);

			return -1;
		} else {
			*error = RIOSOCKETS_ERROR_SOCKET_CREATION;

			return -1;
		}

		create:

		return (RioSocket)rio;
	}

	void riosockets_destroy(RioSocket* socket) {
		Rio* rio = (Rio*)*socket;

		if (rio->socket > 0) {
			rio->functions.RIODeregisterBuffer(rio->sendBuffers[0].data.BufferId);
			rio->functions.RIODeregisterBuffer(rio->sendBuffers[0].address.BufferId);
			rio->functions.RIODeregisterBuffer(rio->receiveBuffers[0].data.BufferId);
			rio->functions.RIODeregisterBuffer(rio->receiveBuffers[0].address.BufferId);

			rio->functions.RIOCloseCompletionQueue(rio->sendQueue);
			rio->functions.RIOCloseCompletionQueue(rio->receiveQueue);

			WSACloseEvent(rio->sendEvent);
			WSACloseEvent(rio->receiveEvent);

			closesocket(rio->socket);

			riosockets_buffer_free(rio->sendMemory);
			riosockets_buffer_free(rio->sendMemoryAddress);
			riosockets_buffer_free(rio->receiveMemory);
			riosockets_buffer_free(rio->receiveMemoryAddress);

			free(rio->sendBuffers);
			free(rio->receiveBuffers);

			free(rio);

			*socket = 0;
		}
	}

	int riosockets_bind(RioSocket socket, const RioAddress* address) {
		Rio* rio = (Rio*)socket;

		if (rio->socket < 1)
			return -1;

		struct sockaddr_in6 socketAddress = { 0 };

		socketAddress.sin6_family = AF_INET6;

		if (address == NULL) {
			socketAddress.sin6_addr = in6addr_any;
			socketAddress.sin6_port = 0;
		} else {
			socketAddress.sin6_addr = address->ipv6;
			socketAddress.sin6_port = RIOSOCKETS_HOST_TO_NET_16(address->port);
		}

		return bind(rio->socket, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
	}

	int riosockets_connect(RioSocket socket, const RioAddress* address) {
		Rio* rio = (Rio*)socket;

		if (rio->socket < 1)
			return -1;

		struct sockaddr_in6 socketAddress = { 0 };

		socketAddress.sin6_family = AF_INET6;
		socketAddress.sin6_addr = address->ipv6;
		socketAddress.sin6_port = RIOSOCKETS_HOST_TO_NET_16(address->port);

		return connect(rio->socket, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
	}

	RioStatus riosockets_set_option(RioSocket socket, int level, int optionName, const int* optionValue, int optionLength) {
		Rio* rio = (Rio*)socket;

		if (rio->socket > 0 && setsockopt(rio->socket, level, optionName, (const char*)optionValue, optionLength) == 0)
			return RIOSOCKETS_STATUS_OK;
		else
			return RIOSOCKETS_STATUS_ERROR;
	}

	RioStatus riosockets_get_option(RioSocket socket, int level, int optionName, int* optionValue, int* optionLength) {
		Rio* rio = (Rio*)socket;

		if (rio->socket > 0 && getsockopt(rio->socket, level, optionName, (char*)optionValue, (socklen_t*)optionLength) == 0)
			return RIOSOCKETS_STATUS_OK;
		else
			return RIOSOCKETS_STATUS_ERROR;
	}

	uint8_t* riosockets_buffer(RioSocket socket, const RioAddress* address, int dataLength) {
		Rio* rio = (Rio*)socket;

		if (rio->socket < 1 || rio->sendBufferPending == rio->sendBufferCount || dataLength > rio->maxBufferLength)
			return NULL;

		if (rio->sendBufferTail == rio->sendBufferCount)
			rio->sendBufferTail = 0;

		uint8_t* buffer = (uint8_t*)(rio->sendMemory + rio->sendBufferTail * rio->maxBufferLength);

		rio->sendBuffers[rio->sendBufferTail].data.Length = dataLength;

		if (address == NULL) {
			rio->sendBuffers[rio->sendBufferTail].addressless = TRUE;
		} else {
			struct sockaddr_in6 socketAddress = { 0 };

			socketAddress.sin6_family = AF_INET6;
			socketAddress.sin6_addr = address->ipv6;
			socketAddress.sin6_port = RIOSOCKETS_HOST_TO_NET_16(address->port);

			struct sockaddr_in6* destinationAddress = (struct sockaddr_in6*)(rio->sendMemoryAddress + rio->sendBufferTail * sizeof(SOCKADDR_INET));

			*destinationAddress = socketAddress;

			rio->sendBuffers[rio->sendBufferTail].addressless = FALSE;
		}

		++rio->sendBufferQueue;
		++rio->sendBufferTail;

		return buffer;
	}

	void riosockets_send(RioSocket socket) {
		Rio* rio = (Rio*)socket;

		if (rio->socket > 0) {
			while (rio->sendBufferQueue != 0) {
				int sendBufferHead = rio->sendBufferTail - rio->sendBufferQueue;

				if (!rio->functions.RIOSendEx(rio->requestQueue, &rio->sendBuffers[sendBufferHead].data, 1, NULL, (rio->sendBuffers[sendBufferHead].addressless == FALSE ? &rio->sendBuffers[sendBufferHead].address : NULL), NULL, NULL, 0, 0)) {
					RioAddress address = { 0 };

					if (rio->sendBuffers[sendBufferHead].addressless == FALSE)
						riosockets_address_extract(&address, (struct sockaddr_storage*)(rio->sendMemoryAddress + sendBufferHead * sizeof(SOCKADDR_INET)));

					rio->callback(socket, &address, (uint8_t*)(rio->sendMemory + sendBufferHead * rio->maxBufferLength), rio->sendBuffers[sendBufferHead].data.Length, RIOSOCKETS_TYPE_SEND);
				} else {
					++rio->sendBufferPending;
				}

				--rio->sendBufferQueue;
			}

			RIORESULT results[RIOSOCKETS_MAX_COMPLETION_RESULTS] = { 0 };
			int completionCount = rio->functions.RIODequeueCompletion(rio->sendQueue, results, rio->sendBufferCount);

			rio->sendBufferPending -= completionCount;
		}
	}

	void riosockets_receive(RioSocket socket, int maxCompletions) {
		Rio* rio = (Rio*)socket;

		if (rio->socket > 0 && maxCompletions > 0) {
			if (maxCompletions > rio->receiveBufferCount)
				maxCompletions = rio->receiveBufferCount;

			if (maxCompletions > RIOSOCKETS_MAX_COMPLETION_RESULTS)
				maxCompletions = RIOSOCKETS_MAX_COMPLETION_RESULTS;

			RIORESULT results[RIOSOCKETS_MAX_COMPLETION_RESULTS] = { 0 };
			int completionCount = rio->functions.RIODequeueCompletion(rio->receiveQueue, results, maxCompletions);

			if (completionCount > 0) {
				for (int i = 0; i < completionCount; i++) {
					RioAddress address = { 0 };

					riosockets_address_extract(&address, (struct sockaddr_storage*)(rio->receiveMemoryAddress + rio->receiveBufferHead * sizeof(SOCKADDR_INET)));

					rio->callback(socket, &address, (uint8_t*)(rio->receiveMemory + rio->receiveBufferHead * rio->maxBufferLength), results[i].BytesTransferred, RIOSOCKETS_TYPE_RECEIVE);
					rio->functions.RIOReceiveEx(rio->requestQueue, &rio->receiveBuffers[rio->receiveBufferHead].data, 1, NULL, &rio->receiveBuffers[rio->receiveBufferHead].address, NULL, NULL, 0, 0);

					++rio->receiveBufferHead;

					if (rio->receiveBufferHead == rio->receiveBufferCount)
						rio->receiveBufferHead = 0;
				}
			}
		}
	}

	RioStatus riosockets_address_get(RioSocket socket, RioAddress* address) {
		Rio* rio = (Rio*)socket;

		if (rio->socket < 1)
			return RIOSOCKETS_STATUS_ERROR;

		struct sockaddr_storage addressStorage = { 0 };
		socklen_t addressLength = sizeof(addressStorage);

		if (getsockname(rio->socket, (struct sockaddr*)&addressStorage, &addressLength) == -1)
			return RIOSOCKETS_STATUS_ERROR;

		riosockets_address_extract(address, &addressStorage);

		return RIOSOCKETS_STATUS_OK;
	}

	RioStatus riosockets_address_is_equal(const RioAddress* left, const RioAddress* right) {
		if (memcmp(left, right, sizeof(struct in6_addr)) == 0 && left->port == right->port)
			return RIOSOCKETS_STATUS_OK;
		else
			return RIOSOCKETS_STATUS_ERROR;
	}

	RioStatus riosockets_address_set_ip(RioAddress* address, const char* ip) {
		int type = AF_INET6;
		void* destination = &address->ipv6;

		if (strchr(ip, ':') == NULL) {
			type = AF_INET;

			memset(address, 0, sizeof(address->ipv4.zeros));

			address->ipv4.ffff = 0xFFFF;
			destination = &address->ipv4.ip;
		}

		if (!inet_pton(type, ip, destination))
			return RIOSOCKETS_STATUS_ERROR;

		return RIOSOCKETS_STATUS_OK;
	}

	RioStatus riosockets_address_get_ip(const RioAddress* address, char* ip, int ipLength) {
		if (inet_ntop(AF_INET6, &address->ipv6, ip, ipLength) == NULL)
			return RIOSOCKETS_STATUS_ERROR;

		if (riosockets_array_is_zeroed(address->ipv4.zeros, sizeof(address->ipv4.zeros)) == 0 && address->ipv4.ffff == 0xFFFF)
			riosockets_string_copy(ip, ip + 7, ipLength);

		return RIOSOCKETS_STATUS_OK;
	}

	RioStatus riosockets_address_set_hostname(RioAddress* address, const char* name) {
		struct addrinfo addressInfo = { 0 }, *result = NULL, *resultList = NULL;

		addressInfo.ai_family = AF_UNSPEC;

		if (getaddrinfo(name, NULL, &addressInfo, &resultList) != 0)
			return RIOSOCKETS_STATUS_ERROR;

		for (result = resultList; result != NULL; result = result->ai_next) {
			if (result->ai_addr != NULL && result->ai_addrlen >= sizeof(struct sockaddr_in)) {
				if (result->ai_family == AF_INET) {
					struct sockaddr_in* socketAddress = (struct sockaddr_in*)result->ai_addr;

					memset(address, 0, sizeof(address->ipv4.zeros));

					address->ipv4.ffff = 0xFFFF;
					address->ipv4.ip.s_addr = socketAddress->sin_addr.s_addr;

					freeaddrinfo(resultList);

					return RIOSOCKETS_STATUS_OK;
				} else if (result->ai_family == AF_INET6) {
					struct sockaddr_in6* socketAddress = (struct sockaddr_in6*)result->ai_addr;

					address->ipv6 = socketAddress->sin6_addr;

					freeaddrinfo(resultList);

					return RIOSOCKETS_STATUS_OK;
				}
			}
		}

		if (resultList != NULL)
			freeaddrinfo(resultList);

		return riosockets_address_set_ip(address, name);
	}

	RioStatus riosockets_address_get_hostname(const RioAddress* address, char* name, int nameLength) {
		struct sockaddr_in6 socketAddress = { 0 };

		socketAddress.sin6_family = AF_INET6;
		socketAddress.sin6_addr = address->ipv6;
		socketAddress.sin6_port = RIOSOCKETS_HOST_TO_NET_16(address->port);

		int error = getnameinfo((struct sockaddr*)&socketAddress, sizeof(socketAddress), name, nameLength, NULL, 0, NI_NAMEREQD);

		if (!error) {
			if (name != NULL && nameLength > 0 && !memchr(name, '\0', nameLength))
				return RIOSOCKETS_STATUS_ERROR;

			return RIOSOCKETS_STATUS_OK;
		}

		if (error != EAI_NONAME)
			return RIOSOCKETS_STATUS_ERROR;

		return riosockets_address_get_ip(address, name, nameLength);
	}

#endif // RIOSOCKETS_IMPLEMENTATION

#endif // RIOSOCKETS_H