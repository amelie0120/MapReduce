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
	Node *map_big;
	char letter;
} ReduceArg;

typedef struct {
    int id;
    char *filename;
    Node *map;
    pthread_mutex_t mutex;
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
	int semafor;
	int number_pairs;
	int start_reducer;
	char find_letter;

	/*
	 * Head of queue used to store tasks.
	 * First item is head.next, if head.next != head (i.e. if queue
	 * is not empty).
	 * Last item is head.prev, if head.prev != head (i.e. if queue
	 * is not empty).
	 */
	os_list_node_t head;

	/* TODO: Define threapool / queue synchronization data. */
	pthread_mutex_t mutex;
	//pthread_mutex_t mutex_semafor;
	pthread_mutex_t mutex_access_map;
	pthread_mutex_t mutex_start;
    pthread_cond_t condition;
	pthread_cond_t cond_start_map;
	//pthread_cond_t cond_start_reduce;
	
	//pthread_cond_t cond_start_reduce;
} os_threadpool_t;

//char *strdup(const char *s)
FileNode *create_file_node(int nr);
// Node *insert_sorted(Node **root, char *word, int file);
FileNode *insert_last(FileNode **node, FileNode *file);
Node *create_node(char *key, FileNode *value);
Node *insert(Node **root, char *key, FileNode *value, int reduce);

os_task_t *create_task(void (*f)(void *), void *arg, void (*destroy_arg)(void *));
void destroy_task(os_task_t *t);

os_threadpool_t *create_threadpool(unsigned int num_threads_M, unsigned int num_threads_R, int nr_fisiere);
void destroy_threadpool(os_threadpool_t *tp);

void enqueue_task(os_threadpool_t *q, os_task_t *t);
os_task_t *dequeue_task(os_threadpool_t *tp);
os_task_t *dequeue_task2(os_threadpool_t *tp);
void wait_for_completion(os_threadpool_t *tp);

#endif
