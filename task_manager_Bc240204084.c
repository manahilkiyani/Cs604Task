
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    char studentID[50];
    int times[3];
    int pipe1[2], pipe2[2];
    int results[3];

    printf("Enter Student ID: ");
    scanf("%49s", studentID);

    for (int i = 0; i < 3; i++) {
        do {
            printf("Enter Task %d Execution Time: ", i + 1);
            if (scanf("%d", &times[i]) != 1) {
                printf("Invalid input!\n");
                return 1;
            }
            if (times[i] <= 0)
                printf("Enter a positive integer!\n");
        } while (times[i] <= 0);
    }

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Pipe error");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork error");
        return 1;
    }

    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        int received[3];

        if (read(pipe1[0], received, sizeof(received))
            != sizeof(received))
            exit(1);

        close(pipe1[0]);

        int total = received[0] + received[1] + received[2];
        int maxTime = received[0];
        int longestTask = 1;

        for (int i = 1; i < 3; i++) {
            if (received[i] > maxTime) {
                maxTime = received[i];
                longestTask = i + 1;
            }
        }

        results[0] = total;
        results[1] = longestTask;
        results[2] = maxTime;

        write(pipe2[1], results, sizeof(results));

        close(pipe2[1]);
        exit(0);
    }

    else {
        close(pipe1[0]);
        close(pipe2[1]);

        write(pipe1[1], times, sizeof(times));
        close(pipe1[1]);

        if (read(pipe2[0], results, sizeof(results))
            != sizeof(results)) {
            perror("Read error");
            close(pipe2[0]);
            waitpid(pid, NULL, 0);
            return 1;
        }

        close(pipe2[0]);
        waitpid(pid, NULL, 0);

        char report[1000];

        int length = snprintf(report, sizeof(report),
            "\n--- Task Processing Report ---\n"
            "Student ID: %s\n"
            "Task 1: %d units\n"
            "Task 2: %d units\n"
            "Task 3: %d units\n"
            "Total Execution Time: %d units\n"
            "Longest Task: Task %d\n"
            "Maximum Execution Time: %d units\n",
            studentID, times[0], times[1], times[2],
            results[0], results[1], results[2]);

        printf("%s", report);

        char filename[100];
        snprintf(filename, sizeof(filename),
                 "%s_report.txt", studentID);

        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd < 0) {
            perror("File error");
            return 1;
        }

        if (write(fd, report, length) != length) {
            perror("Write error");
            close(fd);
            return 1;
        }

        close(fd);

        printf("Report saved to: %s\n", filename);
    }

    return 0;
}
