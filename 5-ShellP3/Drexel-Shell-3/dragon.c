#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>   // For dirname()
#include <unistd.h>   // For readlink()
#include <string.h>

#define PATH_MAX 4096
#define DRAGON "drexel_dragon.txt"

// EXTRA CREDIT - print the Drexel dragon from the readme.md
extern void print_dragon(){
    char exe_path[PATH_MAX];
    ssize_t len;

    // Read the symbolic link /proc/self/exe to get the executable's path
    len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    exe_path[len] = '\0'; 

    // Extract the directory from the executable's path
    char *dir = dirname(exe_path);

    // Construct the absolute path to drexel_dragon.txt
    strcat(dir, "/");
    strcat(dir, DRAGON);

    // Open the drexel_dragon.txt file
    FILE *file = fopen(dir, "r");
    if (file == NULL) {
        printf("Error: Could not open %s: %s\n", DRAGON, dir);
        return;
    }

    // Read and print the file contents line by line
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }
    printf("\n"); // Add an extra newline for formatting

    // Close the file
    fclose(file);
}