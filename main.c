#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

int exit_now = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *http_thread(void *arg)
{
    int sockfd = *((int *)arg);
    int client_sock;

    while(!exit_now) {
        char buf[1024];
        int read_size;

        pthread_mutex_lock(&lock);
        puts("Waiting for incoming connections...");
        client_sock = accept(sockfd, NULL, NULL);
        pthread_mutex_unlock(&lock);

        puts("Connection accepted");
        puts("====================\n");
        while ((read_size = recv(client_sock, buf, 1024 , 0)) > 0) {
            puts(buf);
            if (read_size < 1024) break;
        }
        puts("\n====================");

        if (read_size == -1) {
            perror("recv failed");
        }

        char res[] = "HTTP/1.1 200 OK\n";
        write(client_sock, res, sizeof(res));

        fflush(stdout);
        close(client_sock);
        puts("Client disconnected");
    }
    return NULL;
}

void sighandler (int signo)
{
    exit_now = 1;
    printf("Exit... Wait threads...\n");
    exit(0);
}


int main(int argc, char *argv[])
{
    int N = 1; // кол-во потоков
    uint16_t port = 9999;
    int c;
    while ((c = getopt(argc, argv, "n:p:")) != -1) {
        switch (c) {
            case 'n':
                N = atoi(optarg);
                break;
            case 'p':
                port = (uint16_t) atoi(optarg);
                break;
            default:
                printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    // Этап 1: создаем сокет
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Этап 2: создаем адрес
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    // Этап 3: связываем сокет с адресом
    int err = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));

    if (err != 0)
        perror("Bind error");
    else
        printf("Bind is ok\n");

    // Этап 4: создаем очередь запросов на соединение
    err = listen(sockfd, 5 * N);

    if (err != 0)
        perror("Listen error");
    else
        printf("Listen is ok\n");

    pthread_t *http = malloc(sizeof(pthread_t) * N);
    for (int i = 0; i < N; ++i) {
        if (pthread_create (&http[i], NULL, http_thread, &sockfd) != 0) {
            return EXIT_FAILURE;
        }
    }

    signal(SIGINT, sighandler);
    for (int i = 0; i < N; ++i) {
        if (pthread_join(http[i], NULL) != 0) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
