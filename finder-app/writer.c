#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <message>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *message = argv[2];

    openlog("writer_logs", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Entry with file: %s and message: %s", filename, message);

    FILE *fp = fopen(filename, "a");
    if (fp == NULL) {
        syslog(LOG_ERR, "Failed to open file: %s", filename);
        perror("fopen");
        closelog();
        return 1;
    }

    fprintf(fp, "%s\n", message);
    syslog(LOG_DEBUG, "Writing %s to %s", message, filename);

    if (0 != fclose(fp)) {
	syslog(LOG_ERR, "Error closing file: %s", filename);
	perror("fclose");
        closelog();
        return 1;
    } else {
	syslog(LOG_INFO, "File closed successfully");
    }

    closelog();

    syslog(LOG_INFO, "Exit for file: %s and message: %s", filename, message);
    return 0;
}

