/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef __OS_THREADPOOL_H__
#define __OS_THREADPOOL_H__	1

#include <pthread.h>
#include <string.h>
#include "os_list.h"

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
	os_list_node_t list;
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

	os_list_node_t head;

	pthread_mutex_t mutex;
	pthread_mutex_t mutex_start_reduce;
	pthread_mutex_t mutex_access_map;
	pthread_mutex_t mutex_start;
    pthread_cond_t condition;
	pthread_cond_t cond_start_map;

	pthread_barrier_t bariera;
	pthread_cond_t cond_start_reduce;
	
} os_threadpool_t;

FileNode *create_file_node(int nr);
FileNode *insert_file(FileNode **node, FileNode *file);
Node *create_node(char *key, FileNode *value);
Node *insert(Node **root, char *key, FileNode *value, int map);

os_task_t *create_task(void (*f)(void *), void *arg, void (*destroy_arg)(void *));
void destroy_task(os_task_t *t);

os_threadpool_t *create_threadpool(unsigned int num_threads_M, unsigned int num_threads_R, int nr_fisiere);
void destroy_threadpool(os_threadpool_t *tp);

void enqueue_task(os_threadpool_t *q, os_task_t *t);
os_task_t *dequeue_task(os_threadpool_t *tp);
os_task_t *dequeue_task2(os_threadpool_t *tp);
void wait_for_completion(os_threadpool_t *tp);

#endif
