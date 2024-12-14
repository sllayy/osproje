//kontroller yapıldı, kod denendi, kod çalışıyor (b221210383)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "program.h"



// Kabuk yardım mesajı
int kocsh_help(char** args) {
    printf("   ProgramShell (program):\n");
    printf("1. Tekli komutlar çalıştırabilir.\n");
    printf("2. Giriş ve çıkış yönlendirme yapabilir.\n");
    printf("3. Arka planda çalıştırma desteği vardır (&).\n");
    printf("4. Pipe (|) desteği vardır.\n");
    printf("5. Quit komutu ile çıkabilirsiniz.\n");
    return 1;
}

// Kabuktan çıkış
int kocsh_quit(char** args) {
    printf("Kabuk kapatılıyor...\n");
    exit(0);
}

// Komut istemini göster
void prompt() {
    printf("> ");
    fflush(stdout);
}

// Komut satırını yorumla
char** komutYorumla(char* line) {
    char** args = malloc(MAX_ARGS * sizeof(char*));
    char* arg;
    int index = 0;

    arg = strtok(line, " \t\r\n");
    while (arg != NULL) {
        args[index++] = arg;
        arg = strtok(NULL, " \t\r\n");
    }
    args[index] = NULL;
    return args;
}

// Giriş yönlendirme
int girisYonlendirme(char** args) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Hata: Giriş dosyası belirtilmemiş\n");
                return 0;
            }

            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("Giriş dosyası bulunamadı");
                return 0;
            }

            dup2(fd, STDIN_FILENO);
            close(fd);

            args[i] = NULL; // Yönlendirme sembollerini temizle
            return 1;
        }
        i++;
    }
    return 1; // Giriş yönlendirme yoksa devam et
}

// Çıkış yönlendirme
int cikisYonlendirme(char** args) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Hata: Çıkış dosyası belirtilmemiş\n");
                return 0;
            }

            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Çıkış dosyası açılamadı");
                return 0;
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);

            args[i] = NULL; // Yönlendirme sembollerini temizle
            return 1;
        }
        i++;
    }
    return 1; // Çıkış yönlendirme yoksa devam et
}

// Arka planda çalışma işlevi
int arkaPlandaCalistir(char** args) {
    pid_t pid = fork();
    if (pid == 0) {
        if (!girisYonlendirme(args) || !cikisYonlendirme(args)) {
            exit(EXIT_FAILURE);
        }
        if (execvp(args[0], args) == -1) {
            perror("Komut çalıştırılamadı");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid < 0) {
        perror("Fork başarısız");
    }
    else {
        printf("[%d] arka planda çalışıyor\n", pid);
    }
    return 1;
}

// Pipe desteği
int boruCalistir(char** args) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Pipe oluşturulamadı");
        return 0;
    }

    int i = 0;
    while (args[i] != NULL && strcmp(args[i], "|") != 0) {
        i++;
    }
    if (args[i] == NULL) {
        fprintf(stderr, "Hata: Pipe sembolü eksik\n");
        return 0;
    }

    args[i] = NULL; // Pipe sembolünü temizle
    char** args1 = args;
    char** args2 = &args[i + 1];

    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(args1[0], args1);
        perror("Komut çalıştırılamadı");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(args2[0], args2);
        perror("Komut çalıştırılamadı");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 1;
}

// Tekli komutları çalıştır
int tekliKomutCalistir(char** args) {
    pid_t pid = fork();
    if (pid == 0) {
        if (!girisYonlendirme(args) || !cikisYonlendirme(args)) {
            exit(EXIT_FAILURE);
        }
        if (execvp(args[0], args) == -1) {
            perror("Komut çalıştırılamadı");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("Fork başarısız");
    }
    else {
        waitpid(pid, NULL, 0);
    }
    return 1;
}

// Çalıştırma işlevi
int calistir(char** args) {
    if (args[0] == NULL) {
        return 1;
    }

    if (strcmp(args[0], "help") == 0) return kocsh_help(args);
    if (strcmp(args[0], "quit") == 0) return kocsh_quit(args);

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            return boruCalistir(args);
        }
    }

    int i = 0;
    while (args[i] != NULL) i++;
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        args[i - 1] = NULL;
        return arkaPlandaCalistir(args);
    }

    return tekliKomutCalistir(args);
}

// Çocuk süreç sinyal yöneticisi
void sig_chld(int signo) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("[%d] retval: %d\n", pid, WEXITSTATUS(status));
    }
}

// Ana fonksiyon
int main() {
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    char** args;

    signal(SIGCHLD, sig_chld);

    while (1) {
        prompt();
        nread = getline(&line, &len, stdin);
        if (nread == -1) {
            perror("Hata: Giriş okunamadı");
            break;
        }

        args = komutYorumla(line);
        if (args[0] != NULL) {
            calistir(args);
        }

        free(args);
    }

    free(line);
    return 0;
}
