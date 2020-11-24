#define _WIN32_WINNT 0x0A00

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>

#define DEFAULT_PORT "27015"

#pragma comment(lib, "Ws2_32.lib")

void print(std::string input){
    std::cout << input << std::endl;
    return;
}

void handle_client(SOCKET s){
    print("Socket connected");
    int byte_recv = 0;
    int byte_sent = 0;
    // allocate 512 byte buffer for recv()
    char recieve_buffer[512];
    do {
        byte_recv = recv(s, recieve_buffer, sizeof(recieve_buffer), 0);
        if (byte_recv > 0) {
            std::cout << "Byte recieved: " << byte_recv << std::endl;

            for (auto c: recieve_buffer){
                std::cout << c;
            }
            std::cout << std::endl;
            
            // Echo the buffer back to the sender
            byte_sent = send(s, recieve_buffer, byte_recv, 0);
            if (byte_sent == SOCKET_ERROR) {
                std::cout << "Send error: " << WSAGetLastError() << std::endl;
                closesocket(s);
                return;
            }
            std::cout << "Byte sent: " << byte_sent << std::endl;
        } else if (byte_recv == 0){
            std::cout << "Closing connection..." << std::endl;
            closesocket(s);
        } else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(s);
            return;
        }
    } while (byte_recv > 0);
}

int main(){
    // Main object of this library
    // The WSADATA structure contains information about the Windows Sockets implementation.
    WSAData wsaData;
    
    // The WSAStartup function initiates use of the Winsock DLL by a process.
    int error_code;
    error_code = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (error_code != 0){ std::cout << "Initialisation failed, exiting..." << std::endl; return 1; }

    // create 2 addrinfo pointer that currently store nothing (pre-allocating) and an addrinfo structure that contain information about the address
    addrinfo *result = NULL, *pAddrinfo = NULL, address_info;

    // empty memory in this part
    ZeroMemory(&address_info, sizeof(address_info));
    address_info.ai_family = AF_INET;
    address_info.ai_socktype = SOCK_STREAM;
    address_info.ai_protocol = IPPROTO_TCP;
    address_info.ai_flags = AI_PASSIVE;

    // get, resolve host address then add it to an addrinfo linked list
    error_code = getaddrinfo(NULL, DEFAULT_PORT, &address_info, &result);
    if (error_code != 0){
        print("getaddrinfo() failed, cleaning up...");
        WSACleanup();
        return 1;
    }

    
    SOCKET server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (server_socket == INVALID_SOCKET){
        print("socket initialisation failed: ");
        std::cout << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // for (char c : result->ai_addr->sa_data){
    //     std::cout << c << std::endl;
    // }
    struct sockaddr bind_to_server_socket;
    error_code = bind(server_socket, result->ai_addr, (int)result->ai_addrlen);
    if (error_code == SOCKET_ERROR) {
        print("bind failure: ");
        std::cout << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // once the socket have been bind, the address info "result" was pointing to is no longer needed
    freeaddrinfo(result);

    SOCKET accepted_socket;

    bool running = true;
    while (running){
        int listen_error = 0;
        print("Starting host socket...");
        listen_error = listen(server_socket, SOMAXCONN);
        if (listen_error == SOCKET_ERROR){
            print("Listening failed: ");
            std::cout << WSAGetLastError() << std::endl;
            closesocket(server_socket);
            WSACleanup();
            return 1;
        } else {
            accepted_socket = accept(server_socket, NULL, NULL);
            print("Socket detected");
            handle_client(accepted_socket);
            // std::thread client_thread = std::thread([&]{ handle_client(accepted_socket); });
        }
    }
    return 0;
}