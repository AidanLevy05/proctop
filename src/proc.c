#include "proc.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

int proc_get_list(struct process *list)
{
    DIR *dir = opendir("/proc");
    struct dirent *entry;
    int count = 0;

    if (!dir)
        return 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!isdigit((unsigned char)entry->d_name[0]))
            continue;

        if (count >= MAX_PROCS)
            break;

        int pid = atoi(entry->d_name);
        list[count].pid = pid;
        strcpy(list[count].command, "?");
        strcpy(list[count].user, "?");
        list[count].mem_mb = 0.0;

        char path[256];
        FILE *f;

        sprintf(path, "/proc/%d/comm", pid);
        f = fopen(path, "r");
        if (f)
        {
            if (fgets(list[count].command, sizeof(list[count].command), f))
                list[count].command[strcspn(list[count].command, "\n")] = '\0';
            fclose(f);
        }

        sprintf(path, "/proc/%d/status", pid);
        f = fopen(path, "r");
        if (f)
        {
            char line[256];
            uid_t uid = (uid_t)-1;

            while (fgets(line, sizeof(line), f))
            {
                if (sscanf(line, "Uid:\t%u", &uid) == 1 || sscanf(line, "Uid: %u", &uid) == 1)
                    break;
            }

            fclose(f);

            if (uid != (uid_t)-1)
            {
                struct passwd *pw = getpwuid(uid);
                if (pw)
                {
                    strncpy(list[count].user, pw->pw_name, sizeof(list[count].user) - 1);
                    list[count].user[sizeof(list[count].user) - 1] = '\0';
                }
            }
        }

        sprintf(path, "/proc/%d/statm", pid);
        f = fopen(path, "r");
        if (f)
        {
            long size, resident;

            if (fscanf(f, "%ld %ld", &size, &resident) == 2)
            {
                long page_size = sysconf(_SC_PAGESIZE);
                list[count].mem_mb = (resident * page_size) / (1024.0 * 1024.0);
            }

            fclose(f);
        }

        count++;
    }

    closedir(dir);
    return count;
}
