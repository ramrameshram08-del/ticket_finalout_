#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smart_shuttle.h"

#ifdef _WIN32
  #include <winsock2.h>
  #include <io.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef int socklen_t;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #define SOCKET int
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  #define closesocket close
  #define _dup dup
  #define _dup2 dup2
  #define _fileno fileno
#endif


// URL decoding function
void urldecode(char *src, char *dest) {
    char p[3];
    int code;
    while(*src) {
        if(*src == '%') {
            if(src[1] && src[2]) {
                p[0] = src[1];
                p[1] = src[2];
                p[2] = '\0';
                sscanf(p, "%x", &code);
                *dest++ = (char)code;
                src += 3;
            } else {
                *dest++ = *src++;
            }
        } else if(*src == '+') {
            *dest++ = ' ';
            src++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

void parse_query_params(char* query, char* param_name, char* param_value) {
    char *start = strstr(query, param_name);
    if(start) {
        start += strlen(param_name);
        char *end = strchr(start, '&');
        if(!end) end = start + strlen(start);
        int len = end - start;
        char temp[512] = {0};
        strncpy(temp, start, len);
        urldecode(temp, param_value);
    } else {
        param_value[0] = '\0';
    }
}

void handle_client(SOCKET client) {
    char buffer[4096];
    int recv_size = recv(client, buffer, sizeof(buffer)-1, 0);
    if(recv_size <= 0) return;
    buffer[recv_size] = '\0';

    char method[16], path[1024];
    sscanf(buffer, "%15s %1023s", method, path);

    char* json_header = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache, no-store, must-revalidate\r\nConnection: close\r\n\r\n";
    char* html_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";

    if(strncmp(path, "/api/", 5) == 0) {
        send(client, json_header, strlen(json_header), 0);
        
        FILE *temp = tmpfile();
        int stdout_fd = _dup(1);
        _dup2(_fileno(temp), 1);
        
        if (strstr(path, "/api/routes")) {
            apiGetRoutes();
        } 
        else if (strstr(path, "/api/admin_bookings")) {
            apiGetAllBookings();
        }
        else if (strstr(path, "/api/admin_revenue")) {
            apiGetRevenue();
        }
        else if (strstr(path, "/api/admin_reset")) {
            apiResetBuses();
        }
        else if (strstr(path, "/api/seats")) {
            char busIdStr[16];
            parse_query_params(path, "busId=", busIdStr);
            apiGetSeats(atoi(busIdStr));
        }
        else if (strstr(path, "/api/book")) {
            char busId[16], seatNo[16], rollNo[32], name[64];
            parse_query_params(path, "busId=", busId);
            parse_query_params(path, "seatNo=", seatNo);
            parse_query_params(path, "rollNo=", rollNo);
            parse_query_params(path, "name=", name);
            apiBookSeat(atoi(busId), atoi(seatNo), rollNo, name);
        }
        else if (strstr(path, "/api/tickets")) {
            char rollNo[32];
            parse_query_params(path, "rollNo=", rollNo);
            apiGetTickets(rollNo);
        }
        else if (strstr(path, "/api/cancel")) {
            char ticketId[32];
            parse_query_params(path, "ticketId=", ticketId);
            apiCancelTicket(atoi(ticketId));
        }
        else if (strstr(path, "/api/admin_login")) {
            char password[64];
            parse_query_params(path, "password=", password);
            apiVerifyAdmin(password);
        }
        else if (strstr(path, "/api/register")) {
            char rollNo[32], name[64];
            parse_query_params(path, "rollNo=", rollNo);
            parse_query_params(path, "name=", name);
            apiRegisterStudent(rollNo, name);
        }
        else if (strstr(path, "/api/verify_login")) {
            char rollNo[32], name[64];
            parse_query_params(path, "rollNo=", rollNo);
            parse_query_params(path, "name=", name);
            apiVerifyLogin(rollNo, name);
        }
        else if (strstr(path, "/api/admin_pending")) {
            apiGetPendingStudents();
        }
        else if (strstr(path, "/api/admin_approve")) {
            char rollNo[32];
            parse_query_params(path, "rollNo=", rollNo);
            apiApproveStudent(rollNo);
        }
        else if (strstr(path, "/api/admin_reject")) {
            char rollNo[32];
            parse_query_params(path, "rollNo=", rollNo);
            apiRejectStudent(rollNo);
        }
        else if (strstr(path, "/api/admin_blacklist")) {
            char rollNo[32];
            parse_query_params(path, "rollNo=", rollNo);
            apiBlacklistStudent(rollNo);
        }
        else if (strstr(path, "/api/admin_unblacklist")) {
            char rollNo[32];
            parse_query_params(path, "rollNo=", rollNo);
            apiUnblacklistStudent(rollNo);
        }
        else if (strstr(path, "/api/admin_all_students")) {
            apiGetAllStudents();
        }
        
        fflush(stdout);
        _dup2(stdout_fd, 1);
        
        rewind(temp);
        char out_buf[1024];
        int n;
        while((n = fread(out_buf, 1, sizeof(out_buf), temp)) > 0) {
            send(client, out_buf, n, 0);
        }
        fclose(temp);
        
    } else {
        const char *filename = "ui.html";
        if(strcmp(path, "/admin") == 0 || strcmp(path, "/admin_ui.html") == 0) {
            filename = "admin_ui.html";
        }
        
        FILE *f = fopen(filename, "rb");
        if(f) {
            send(client, html_header, strlen(html_header), 0);
            char file_buf[4096];
            int n;
            while((n = fread(file_buf, 1, sizeof(file_buf), f)) > 0) {
                send(client, file_buf, n, 0);
            }
            fclose(f);
        } else {
            char* nf = "HTTP/1.1 404 Not Found\r\n\r\nSmart Shuttle Server. File not found.";
            send(client, nf, strlen(nf), 0);
        }
    }
}

void start_web_server() {
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;

#ifdef _WIN32
    WSADATA wsa;
    printf("\nInitializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed. Error Code : %d\n", WSAGetLastError());
        return;
    }
#endif

    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket\n");
        return;
    }

    int port = 8080;
    char *env_port = getenv("PORT");
    if(env_port) {
        port = atoi(env_port);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if(bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        return;
    }

    listen(server_socket, 3);
    printf("========================================\n");
    printf("   C WEB SERVER RUNNING ON PORT %d    \n", port);
    printf("========================================\n");
    printf("Visit: http://localhost:%d/\n", port);
    printf("Press Ctrl+C to exit.\n");

    int c = sizeof(struct sockaddr_in);
    while((client_socket = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&c)) != INVALID_SOCKET) {
        handle_client(client_socket);
        closesocket(client_socket);
    }

    closesocket(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}
