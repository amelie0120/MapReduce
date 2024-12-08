#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "os_threadpool.h"

void process_word(const char *input, char *output) {
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (isalpha(input[i])) { // Verifică dacă este caracter alfabetic
            output[j++] = tolower(input[i]); // Transformă în litere mici
        }
    }
    output[j] = '\0'; // Terminator de șir
}

Node *map(void *arg){
    ThreadArgs *args = (ThreadArgs *)arg;
    FILE *file = fopen(args->filename, "r");
    if (file == NULL){
        perror("Error opening file");
        exit(1);
    }
    int checkpoint = 0;

    char buffer[1024];
    char word[256];
    char leftover[256] = "";
    //printf("File %s opened\n", args->filename);
    //args->hashmap = malloc(sizeof(args->hashmap));

    // Read the file line by line
    while (fgets(buffer, sizeof(buffer), file)) {
         if (strlen(leftover) > 0) {
            memmove(buffer + strlen(leftover), buffer, strlen(buffer) + 1); // Shift buffer content
            strncpy(buffer, leftover, strlen(leftover));                   // Copy leftover to buffer
            leftover[0] = '\0';                                            // Clear leftover
        }

        int len = strlen(buffer);
        int start = 0;

        // Parcurge linia pentru a extrage cuvintele
        for (int i = 0; i <= len; i++) {
            if (isspace(buffer[i])) {
                // Găsit spațiu sau finalul liniei
                if (i > start) {
                    char temp[256];
                    strncpy(temp, &buffer[start], i - start); // Copiază cuvântul
                    temp[i - start] = '\0';
                    process_word(temp, word); // Procesează cuvântul
                    if (strlen(word) > 0) {   // Verifică dacă e valid
                        //printf("%s\n", word);
                        // if (args->id == 80)
                        //     printf("%s ", word);
                        
                        FileNode *file = create_file_node(args->id);
                        insert(&args->map, word, file, 1);
                    }
                }
                start = i + 1;
            }
        }
        if (start < len) {
            strncpy(leftover, &buffer[start], len - start); // Save the leftover
            leftover[len - start] = '\0'; // Null-terminate
            // if (args->id == 80)
            //     printf("%s ", leftover);
        }
    }
    // if (args->id == 80)
    //     printf("\n");

    if (strlen(leftover) > 0) {
        process_word(leftover, word); // Process the word

        if (strlen(word) > 0) {
            FileNode *file = create_file_node(args->id);
            insert(&args->map, word, file, 1);
        }
    }

    // Node *current = args->map;
    // while (current != NULL){
    //     printf("%s: %d\n", current->word, current->files);
    //     current = current->right;
    // }
    // printf("\n");

    
    
    fclose(file);
    //printf("finished %s\n", args->filename);
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
    

    pthread_mutex_t mutex_index_fisier;
    pthread_mutex_init(&mutex_index_fisier, NULL);
    //argv[3] e filename-ul
    FILE *file = fopen(argv[3], "r");
    if (file == NULL){
        perror("Error opening file");
        exit(1);
    }

    int nr_fisiere;
    if (fgets(buffer, sizeof(buffer), file) != NULL){
        nr_fisiere = atoi(buffer);
        //printf("%d texte\n", nr_fisiere);
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
        // Remove trailing newline, if present
        buffer[strcspn(buffer, "\n")] = '\0';

        lines[lineCount] = malloc(strlen(buffer) + 1);
        if (lines[lineCount] == NULL) {
            perror("Memory allocation failed");
            break;
        }
        strcpy(lines[lineCount], buffer);
        lineCount++;
    }


    pthread_t threads[M + R];
    int r;
    long id;
    void *status;
    int ids[M + R];

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
        //printf("a fct enqueue pt fisieru %d\n", i);
    }    

    //printf("aici?\n");
    wait_for_completion(tp);
    destroy_threadpool(tp);

    fclose(file);
    return 0;
}