#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SECTION_MAX 100
#define SECTION_COUNT 10

typedef enum
{
    NONE    = 0,
    /* PRE_ENTRY = 1, */
    ACTIVE  = 2,
    LOCKING = 3, // Cannot be influenced anymore
    LOCKED  = 4, // Tetromino is being locked to the playfield.
    ENTRY   = 5,
    IDLE    = 10,
} tap_state;

bool isInPlayingState(tap_state state)
{
    return state != 0 && state != 10;
}

struct datapoint_t
{
        int level;
        int time;
};

struct section_t
{
        struct datapoint_t data[SECTION_MAX];
        size_t size;
};

struct game_t
{
        struct section_t sections[SECTION_COUNT];

        int singleCount;
        int doubleCount;
        int tripleCount;
        int tetrisCount;
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        perror("This program expects shmupmametgm as its first argument. Each subsequent argument is passed to shmupmametgm.");
        return 1;
    }
    else
    {
        for (int i = 0; i < argc; ++i)
        {
            printf("%s\n", argv[i]);
        }
    }

    int outfd[2];
    int infd[2];

    int oldstdin, oldstdout;

    pipe(outfd); // Where the parent is going to write to
    pipe(infd); // From where parent is going to read

    oldstdin = dup(0); // Save current stdin
    oldstdout = dup(1); // Save stdout

    close(0);
    close(1);

    dup2(outfd[0], 0); // Make the read end of outfd pipe as stdin
    dup2(infd[1], 1); // Make the write end of infd as stdout

    int subprocessPid = fork();
    if (subprocessPid < 0) // Uh oh
    {
        perror("Could not fork process.");
        exit(1);
    }
    else if (subprocessPid == 0) // Child
    {
        // Child doesn't need to receive input from us (the parent).
        close(outfd[0]);
        close(outfd[1]);
        close(infd[0]);
        close(infd[1]);

        execv(argv[1], argv + 1);
    }
    else // Parent
    {
        // Restore the original std fds of parent
        close(0);
        close(1);
        dup2(oldstdin, 0);
        dup2(oldstdout, 1);

        close(outfd[0]); // These are being used by the child
        close(infd[1]);

        const int bufsize = 256;
        char buffer[bufsize];

        int incomingFD = infd[0];

        while (true)
        {
            // Check if child process has ended.
            int status = 0;
            if (waitpid(subprocessPid, &status, WNOHANG))
            {
                break;
            }

            buffer[read(incomingFD, buffer, bufsize)] = 0;

            if (strlen(buffer))
            {
                // Tokenize the input (formatted as: state level time)
                char* token = strtok(buffer, " \n");
                int state, level, time;

                do
                {
                    state = atoi(token);
                    level = atoi(strtok(NULL, " \n"));
                    time  = atoi(strtok(NULL, " \n"));

                    token = strtok(NULL, " \n");
                } while (token);

                printf("state: %d, level %d, time %d\n", state, level, time);
            }
        }
    }
    return 0;
}
