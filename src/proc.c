#include "proc.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

long long get_total_cpu_time(void) {
    FILE *f = fopen("/proc/stat", "r");
    long long user, nice, system, idle, iowait, irq, softirq, steal;
    long long total = 0;

    if (!f)
        return 0;

    if (fscanf(f, "cpu %lld %lld %lld %lld %lld %lld %lld %lld",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) == 8)
    {
        total = user + nice + system + idle + iowait + irq + softirq + steal;
    }

    fclose(f);
    return total;
}

static int prev_pids[MAX_PROCS];
static long long prev_proc_times[MAX_PROCS];
static int prev_count = 0;
static long long prev_total_cpu = 0;
static int proc_sort_mode = PROC_SORT_MEM;
static char proc_filter[64] = "";

int compare_mem_desc(const void *a, const void *b) {
    const struct process *pa = a;
    const struct process *pb = b;

    if (pb->mem_mb > pa->mem_mb) return 1;
    if (pb->mem_mb < pa->mem_mb) return -1;
    return 0;
}

int compare_pid_asc(const void *a, const void *b) {
    const struct process *pa = a;
    const struct process *pb = b;

    if (pa->pid > pb->pid) return 1;
    if (pa->pid < pb->pid) return -1;
    return 0;
}

int compare_cpu_desc(const void *a, const void *b) {
    const struct process *pa = a;
    const struct process *pb = b;

    if (pb->cpu_percent > pa->cpu_percent) return 1;
    if (pb->cpu_percent < pa->cpu_percent) return -1;
    return compare_mem_desc(a, b);
}

long long find_prev_proc_time(int pid)
{
    for (int i = 0; i < prev_count; i++) {
        if (prev_pids[i] == pid)
            return prev_proc_times[i];
    }

    return -1;
}

void proc_set_sort_mode(int mode)
{
    if (mode == PROC_SORT_MEM || mode == PROC_SORT_PID || mode == PROC_SORT_CPU)
        proc_sort_mode = mode;
}

void proc_set_filter(const char *filter)
{
    if (!filter) {
        proc_filter[0] = '\0';
        return;
    }

    strncpy(proc_filter, filter, sizeof(proc_filter) - 1);
    proc_filter[sizeof(proc_filter) - 1] = '\0';
}

int proc_get_list(struct process *list)
{
    DIR *dir = opendir("/proc");
    struct dirent *entry;
    int count = 0;
    int current_pids[MAX_PROCS];
    long long current_proc_times[MAX_PROCS];
    int current_count = 0;
    long long total_cpu_now = get_total_cpu_time();
    long long total_cpu_delta = total_cpu_now - prev_total_cpu;

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
        list[count].cpu_percent = 0.0;

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

        if (proc_filter[0] != '\0' && !strstr(list[count].command, proc_filter))
            continue;

        sprintf(path, "/proc/%d/stat", pid);
        f = fopen(path, "r");
        if (f)
        {
            char line[1024];

            if (fgets(line, sizeof(line), f))
            {
                char *rest = strrchr(line, ')');

                if (rest)
                {
                    unsigned long utime, stime;
                    int field = 3;

                    rest++;

                    while (field < 14 && rest)
                    {
                        rest = strchr(rest + 1, ' ');
                        field++;
                    }

                    if (rest && sscanf(rest, " %lu %lu", &utime, &stime) == 2)
                    {
                        long long proc_time = (long long)utime + stime;
                        long long prev_proc_time = find_prev_proc_time(pid);

                        current_pids[current_count] = pid;
                        current_proc_times[current_count] = proc_time;
                        current_count++;

                        if (prev_proc_time >= 0 && prev_total_cpu > 0 && total_cpu_delta > 0)
                            list[count].cpu_percent = 100.0 * (proc_time - prev_proc_time) / total_cpu_delta;
                    }
                }
            }

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

    memcpy(prev_pids, current_pids, current_count * sizeof(int));
    memcpy(prev_proc_times, current_proc_times, current_count * sizeof(long long));
    prev_count = current_count;
    prev_total_cpu = total_cpu_now;

    if (proc_sort_mode == PROC_SORT_PID)
        qsort(list, count, sizeof(struct process), compare_pid_asc);
    else if (proc_sort_mode == PROC_SORT_CPU)
        qsort(list, count, sizeof(struct process), compare_cpu_desc);
    else
        qsort(list, count, sizeof(struct process), compare_mem_desc);

    return count;
}
