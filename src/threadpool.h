
#ifndef __OS_THREADPOOL_H__
#define __OS_THREADPOOL_H__	1

#include <pthread.h>
#include <string.h>
#include "list_structures.h"

typedef struct FileNode{
    int nr;
    struct FileNode *prev, *next;
} FileNode;

typedef struct Node {
    char *word;
    FileNode *files;
	int nr_files;
    struct Node *left, *right;
} Node;

typedef struct {
	Node **maps;
	char letter;
	int nr_fisiere;
} ReduceArg;

typedef struct {
    int id;
    char *filename;
    Node *map;
} ThreadArgs;

typedef struct {
	void *argument;
	void (*action)(void *arg);
	void (*destroy_arg)(void *arg);
	queue_list list;
} os_task_t;

typedef struct os_threadpool {
	unsigned int num_threads_M;
	unsigned int num_threads_R;
	pthread_t *threads;
	unsigned int waiting_threads;
	unsigned int finished_threads;
	int start;
	Node *map_big;
	Node **list_of_lists;
	int nr_fisiere;

	queue_list head;

	pthread_mutex_t mutex;
	pthread_mutex_t mutex_start_reduce;
	pthread_mutex_t mutex_access_map;
	pthread_mutex_t mutex_start;

    pthread_cond_t condition;
	pthread_cond_t cond_start_map;
	pthread_cond_t cond_start_reduce;

	pthread_barrier_t bariera;
	
	
} os_threadpool_t;

FileNode *create_file_node(int nr);
Node *create_node(char *word, FileNode *file);
Node *insert_alphabetically(Node **root, char *word, FileNode *file, int map);
Node *insert(Node **root, char *word, FileNode *file, int map);
Node *insert_sorted(Node **root, char *word, FileNode *files);
void *reduce(void *arg);


os_task_t *create_task(void (*f)(void *), void *arg, void (*destroy_arg)(void *));
void destroy_task(os_task_t *t);

os_threadpool_t *create_threadpool(unsigned int num_threads_M, unsigned int num_threads_R, int nr_fisiere);
void destroy_threadpool(os_threadpool_t *tp);

void enqueue_task(os_threadpool_t *q, os_task_t *t);
os_task_t *dequeue_task(os_threadpool_t *tp);
os_task_t *dequeue_task2(os_threadpool_t *tp);
void *functie(void *arg);
void wait_for_completion(os_threadpool_t *tp);


#endif
