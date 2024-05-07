#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define CLIENT_IP "127.0.0.1"
#define SERVER_PORT 8080
#define CLIENT_PORT 8090
#define BUFFER_SIZE 1024


int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Создание raw-сокета
    if ((client_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настройка сокета для использования собственного IP-заголовка
    int optval = 1;
    if (setsockopt(client_socket, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval)) < 0) {
        perror("setsockopt(IP_HDRINCL) failed");
        exit(EXIT_FAILURE);
    }

    // Заполнение адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Создание UDP-заголовка
    struct udphdr udp_header;
    udp_header.source = htons(CLIENT_PORT);
    udp_header.dest = htons(SERVER_PORT);
    udp_header.len = htons(sizeof(udp_header) + strlen("Hello"));
    udp_header.check = 0;

    // Заполнение сообщения
    char message[BUFFER_SIZE];
    strcpy(message, "Hello");

    

    // Создание IP-заголовка
    struct iphdr ip_header;
    ip_header.version = 4;
    ip_header.ihl = 5;
    ip_header.tos = 0;
    ip_header.tot_len = htons(sizeof(ip_header) + sizeof(udp_header) + strlen(message));
    ip_header.id = htons(54321);
    ip_header.frag_off = 0;
    ip_header.ttl = 255;
    ip_header.protocol = IPPROTO_UDP;
    ip_header.check = 0;
    ip_header.saddr = inet_pton(AF_INET, CLIENT_IP, &server_addr.sin_addr);
    ip_header.daddr = inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Формирование полного сообщения с UDP-заголовком
    char packet[BUFFER_SIZE];
    memset(packet, 0, BUFFER_SIZE);   
    memcpy(packet, &ip_header, sizeof(ip_header));
    memcpy(packet + sizeof(ip_header), &udp_header, sizeof(udp_header));
    memcpy(packet + sizeof(ip_header) + sizeof(udp_header), message, strlen(message));

    // Отправка пакета серверу
    if (sendto(client_socket, packet, sizeof(ip_header) + sizeof(udp_header) + strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Sendto failed");
        exit(EXIT_FAILURE);
    }

    printf("Sent: Hello\n");

    // Получение ответа от сервера
    socklen_t len = sizeof(server_addr);
    int received_len = recvfrom(client_socket, (char *)buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &len);

    if (received_len > 0) {
        printf("%d\n", received_len);
        printf("Received: %s\n", buffer + 28);
    }

    close(client_socket);
    return 0;
}
