#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Get RSS (Resident Set Size) in kilobytes from /proc/self/status
long get_rss_kb(void)
{
#ifdef __linux__
    FILE *status = fopen("/proc/self/status", "r");
    if (!status)
        return -1;

    char line[256];
    while (fgets(line, sizeof(line), status))
    {
        if (strncmp(line, "VmRSS:", 6) == 0)
        {
            long rss;
            if (sscanf(line + 6, "%ld", &rss) == 1)
            {
                fclose(status);
                return rss;
            }
        }
    }
    fclose(status);
#endif
    return -1; // Not available on non-Linux systems
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Measure RSS before loading
    long rss_before = get_rss_kb();

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file)
    {
        fprintf(stderr, "Failed to open the file: %s\n", argv[1]);
        return 1;
    }

    json_t *arr = json_array();
    if (!arr)
    {
        fprintf(stderr, "Failed to create JSON array\n");
        fclose(input_file);
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, input_file)) != -1)
    {
        // Remove trailing newline
        if (line[read - 1] == '\n')
            line[read - 1] = '\0';

        json_error_t error;
        json_t *obj = json_loads(line, 0, &error);
        if (!obj)
        {
            fprintf(stderr, "Failed to parse JSON: %s\n", error.text);
            continue;
        }

        json_array_append_new(arr, obj);
    }

    free(line);
    fclose(input_file);

    // Measure RSS after loading
    long rss_after = get_rss_kb();

    if (rss_before >= 0 && rss_after >= 0)
    {
        long rss_diff = rss_after - rss_before;
        printf("jansson RSS: %ld KB\n", rss_diff);
    }
    else
    {
        fprintf(stderr, "RSS measurement not available on this platform\n");
    }

    json_decref(arr);
    return 0;
}
