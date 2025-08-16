#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 5555

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        return 1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        return 1;
    }

    std::vector<int> clients;
    fd_set readfds;

    std::cout << "Server started on port " << PORT << "\n";

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        for (int client : clients) {
            FD_SET(client, &readfds);
            if (client > max_sd) max_sd = client;
        }

        if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("Select error");
            break;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            int new_socket = accept(server_fd, NULL, NULL);
            if (new_socket >= 0) {
                clients.push_back(new_socket);
                std::cout << "New client connected\n";
            }
        }

        for (size_t i = 0; i < clients.size(); ++i) {
            int sd = clients[i];
            if (FD_ISSET(sd, &readfds)) {
                char buffer[1024] = {0};
                int valread = read(sd, buffer, sizeof(buffer));
                if (valread <= 0) {
                    close(sd);
                    clients.erase(clients.begin() + i);
                    std::cout << "Client disconnected\n";
                    --i;
                } else {
                    std::cout << "Message: " << buffer;
                    for (int client : clients) {
                        if (client != sd) {
                            send(client, buffer, strlen(buffer), 0);
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
