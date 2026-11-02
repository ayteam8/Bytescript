#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <bs_api.h>
#include <stdbool.h>
#include <sys/time.h>
#include <netdb.h>
#include "bytescript.h"


int newSocket() {
	int socket_fd = socket(AF_INET,SOCK_STREAM,0);
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
	
	return socket_fd;
}

int Connect(int sock, const char *ip, int port) {
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serv_addr.sin_addr);

	return connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
}

int Send(int sock, char *str) {
	return send(sock, str, strlen(str), 0);
}
int Recv(int sock, char *buffer, int size, bool nblock) {
	return recv(sock, buffer, size, nblock ? MSG_DONTWAIT : 0);
}

int Bind(int sock, int port) {
	struct sockaddr_in serv;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = INADDR_ANY;
	return bind(sock,(struct sockaddr *)&serv,sizeof(serv));
}

int Listen(int sock, int many) {
	return listen(sock, many);
}

int Accept(int sock) {
	int new_socket;
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);

	return accept(sock, (struct sockaddr *)&address, &addrlen);
}

int Close(int sock) {
	return close(sock);
}


TValue native_getip(TValue *argv, size_t argc) {
	MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "net.getip");
	TStr *str = AS_STRING(*argv);

	char *dominio = str->str;
	struct hostent *he = gethostbyname(dominio);
	if (he == NULL) {
		herror("Error resolviendo el host");
		return NUM_AS_TVALUE(-1);
	}

    
	struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    
	if (addr_list[0] != NULL) {
        
		char ip_str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, addr_list[0], ip_str, sizeof(ip_str));
		return make_fstring(ip_str);
	}
    return NUM_AS_TVALUE(-1);
}



TValue native_socket(TValue *args, size_t argc) {
	return NUM_AS_TVALUE(newSocket());
}

TValue native_connect(TValue *args, size_t argc) {
	if (argc < 3) return NUM_AS_TVALUE(-1);
	int sock = args[0].num;
	const char *ip = AS_STRING(args[1])->str;
	int port = args[2].num;

	return NUM_AS_TVALUE(Connect(sock, ip, port));
}

TValue native_send(TValue *args, size_t argc) {
	if (argc < 2) return NUM_AS_TVALUE(-1);
	int sock = args[0].num;
	char *msg = AS_STRING(args[1])->str;
	return NUM_AS_TVALUE(Send(sock, msg));
}

#define BUFFER_SIZE 1024
char s_buffer[1024];

TValue native_recv(TValue *args, size_t argc) {
	if (argc < 2) return NUM_AS_TVALUE(-1);
	int sock = args[0].num;
	bool nblock = ((int)args[1].num==0) ? true : false;
	memset(s_buffer, 0, BUFFER_SIZE);

	int bytes = Recv(sock, s_buffer, BUFFER_SIZE, nblock);
	s_buffer[1024-1] = '\0';
	return make_fstring(s_buffer);
}

TValue native_bind(TValue *args, size_t argc) {
	if (argc < 2) return NUM_AS_TVALUE(-1);
	int sock = args[0].num;
	int port = args[1].num;

	return NUM_AS_TVALUE(Bind(sock, port));
}
TValue native_listen(TValue *args, size_t argc) {
	if (argc < 2) return NUM_AS_TVALUE(-1);
	int sock = args[0].num;
	int num = args[1].num;

	return NUM_AS_TVALUE(Listen(sock, num));
}

TValue native_accept(TValue *args, size_t argc) {
	if (argc < 1) return NUM_AS_TVALUE(-1);
	int sock = args[0].num;

	return NUM_AS_TVALUE(Accept(sock));
}

TValue native_close(TValue *args, size_t argc) {
	if (argc < 1) return NUM_AS_TVALUE(-1);
	int sock = args[0].num;

	return NUM_AS_TVALUE(Close(sock));
}


BS_API char* bytescript_module_map = "net_map.bs";
BS_API TValue bytescript_module_loader(TValue* argv, size_t argc) {
    TValue fields[] = {
        new_cfunc(native_getip),
        new_cfunc(native_socket),
        new_cfunc(native_connect),
        new_cfunc(native_send),
        new_cfunc(native_recv),
        new_cfunc(native_bind),
        new_cfunc(native_listen),
        new_cfunc(native_accept), 
        new_cfunc(native_close)
    };

    size_t field_count = sizeof(fields) / sizeof(fields[0]);
    memcpy(argv, fields, BSIZE(field_count));
}
