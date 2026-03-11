#include "proc.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pwd.h>

int proc_get_list(struct process *list)
{
    DIR *dir = opendir("/proc");
    struct dirent *entry;

    int count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!isdigit(entry->d_name[0]))
            continue;

        int pid = atoi(entry->d_name);

        list[count].pid = pid;

        char path[256];

        /* command name */
        sprintf(path, "/proc/%d/comm", pid);

        FILE *f = fopen(path, "r");

        if (f) {
            fgets(list[count].command, 64, f);
            list[count].command[strcspn(list[count].command, "\n")] = 0;
            fclose(f);
        } else {
            strcpy(list[count].command, "?");
        }

        /* user */
        sprintf(path, "/proc/%d/status", pid);
        f = fopen(path, "r");

        if (f) {

            char line[256];
            int uid = -1;

            while (fgets(line, sizeof(line), f)) {

                if (sscanf(line, "Uid: %d", &uid) == 1)
                    break;
            }

            fclose(f);

            struct passwd *pw = getpwuid(uid);

            if (pw)
                strncpy(list[count].user, pw->pw_name, 31);
            else
                strcpy(list[count].user, "?");

        } else {
            strcpy(list[count].user, "?");
        }

        count++;

        if (count >= MAX_PROCS)
            break;
    }

    closedir(dir);

    return count;
}
