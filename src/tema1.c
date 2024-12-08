#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "os_threadpool.h"

void process_word(const char *input, char *output) {
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (isalpha(input[i])) { 
            output[j++] = tolower(input[i]);
        }
    }
    output[j] = '\0'; 
}

Node *map(void *arg){
    ThreadArgs *args = (ThreadArgs *)arg;
    FILE *file = fopen(args->filename, "r");
    if (file == NULL){
        perror("Error opening file");
        exit(1);
    }

    char buffer[1024];
    char word[256];
    char leftover[256] = "";

    while (fgets(buffer, sizeof(buffer), file)) {
         if (strlen(leftover) > 0) {
            memmove(buffer + strlen(leftover), buffer, strlen(buffer) + 1); 
            strncpy(buffer, leftover, strlen(leftover));                   
            leftover[0] = '\0';                                            
        }

        int len = strlen(buffer);
        int start = 0;

        for (int i = 0; i <= len; i++) {
            if (isspace(buffer[i])) {
                if (i > start) {
                    char temp[256];
                    strncpy(temp, &buffer[start], i - start); 
                    temp[i - start] = '\0';
                    process_word(temp, word);
                    if (strlen(word) > 0) {
                        FileNode *file = create_file_node(args->id);
                        insert(&args->map, word, file, 1);
                    }
                }
                start = i + 1;
            }
        }
        if (start < len) {
            strncpy(leftover, &buffer[start], len - start);
            leftover[len - start] = '\0'; 
        }
    }

    if (strlen(leftover) > 0) {
        process_word(leftover, word);

        if (strlen(word) > 0) {
            FileNode *file = create_file_node(args->id);
            insert(&args->map, word, file, 1);
        }
    }
    
    fclose(file);
    return args->map;
}

int main(int argc, char **argv){
    if (argc < 3){
        perror("Not enough arguments");
        return 1;
    }

    int M = atoi(argv[1]);
    int R = atoi(argv[2]);
    char buffer[256];
    
    FILE *file = fopen(argv[3], "r");
    if (file == NULL){
        perror("Error opening file");
        exit(1);
    }

    int nr_fisiere;
    if (fgets(buffer, sizeof(buffer), file) != NULL){
        nr_fisiere = atoi(buffer);
    }

    char **lines = NULL;
    lines = malloc((nr_fisiere + 1) * sizeof(char *));
    if (lines == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }

    int lineCount = 1;
    while (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\n")] = '\0';

        lines[lineCount] = malloc(strlen(buffer) + 1);
        if (lines[lineCount] == NULL) {
            perror("Memory allocation failed");
            break;
        }
        strcpy(lines[lineCount], buffer);
        lineCount++;
    }

    ThreadArgs **args = malloc((nr_fisiere + 1) * sizeof(ThreadArgs *));
    for (int i = 1; i <= nr_fisiere; i++){
        args[i] = malloc(sizeof(ThreadArgs));
        if (args[i] == NULL) {
            perror("Failed to allocate memory for thread arguments");
            return 1;
        }
    }

    os_threadpool_t *tp = create_threadpool(M, R, nr_fisiere);

    for (int i = 1; i <= nr_fisiere; i++) {
        args[i]->id = i;
        args[i]->filename = lines[i];
        args[i]->map = NULL;
        os_task_t *new_task = create_task((void *)map, args[i], NULL);
        enqueue_task(tp, new_task);
    }    

    wait_for_completion(tp);
    destroy_threadpool(tp);

    // for (int i = 1; i <= nr_fisiere; i++){
    //     free(args[i]);
    // }
    // free(args);

    fclose(file);
    return 0;
}