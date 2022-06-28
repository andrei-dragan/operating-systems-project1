#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int arcg, char **argv) {
    // Select the child
    int child, letter = 0;
    srand(time(NULL));
    child = rand() % 6 + 1;

    // Create the pipes
    int children[7][2];
    for(int i=1;i<=6;i++)
        pipe(children[i]);

    // initialize the first child
    int info[2];
    info[0] = letter;
    info[1] = 1;
    write(children[child][1], info, sizeof(info));

    // create all children
    int p;
    for(child = 1; child <= 6; child++) {
        p = fork();
        if(p == -1) {
            printf("Error creating child number %d!\n", child);
            exit(1);
        }
        else if(p == 0) {

            // get the previous and the next
            int prev_child = child - 1;
            if (prev_child == 0) prev_child = 6;

            int next_child = child + 1;
            if (next_child == 7) next_child = 1;

            // close what is not needed
            for(int child_pipe = 1; child_pipe <= 6; child_pipe++) {
                if (child_pipe != child && child_pipe != next_child && child_pipe != prev_child) {
                    close(children[child_pipe][0]);
                    close(children[child_pipe][1]);
                }
            }

            // we may only write to next_child or prev_child
            close(children[next_child][0]);
            close(children[prev_child][0]);
            // we only read from child
            close(children[child][1]);

            while(1) {
                // wait to read something from the pipe
                int ck_read = read(children[child][0], info, sizeof(info));

                if(info[0] == -1 || ck_read == 0) {
                    // it means that we should close this
                    info[0] = -1;

                    if(info[1] == 1) write(children[next_child][1], info, sizeof(info));
                    else write(children[prev_child][1], info, sizeof(info));
                    break;
                }

                int skip = rand() % 6 + 1;
                if (skip == 1) {
                    info[1] = info[1] * (-1);
                    printf("Child %d having the id of: %d, skipped letter %c\n", child, getpid(), (char)(info[0]+'A'));
                }
                else {
                    printf("Child %d having the id of: %d, has the letter %c\n", child, getpid(), (char)(info[0]+'A'));
                }

                info[0]++;
                if (info[0] == 26) {
                    printf("Child number %d said the last letter of the alphabet\n", child);
                    info[0] = -1;

                    if(info[1] == 1) write(children[next_child][1], info, sizeof(info));
                    else write(children[prev_child][1], info, sizeof(info));

                    break;
                }
                else if (info[0] <= 25) {
                    if (info[1] == 1) write(children[next_child][1], info, sizeof(info));
                    else write(children[prev_child][1], info, sizeof(info));
                }
            }
            close(children[next_child][1]);
            close(children[prev_child][1]);
            close(children[child][0]);
            exit(0);
        }
    }

    for(int child_pipe = 1; child_pipe <= 6; child_pipe++) {
        close(children[child_pipe][0]);
        close(children[child_pipe][1]);
    }

    for(child = 1; child <= 6; child++)
        wait(0);

    return 0;
}