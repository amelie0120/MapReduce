
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "threadpool.h"

#define min(a, b) ((a) < (b) ? (a) : (b))


FileNode *create_file_node(int nr){
	FileNode *file = malloc(sizeof(FileNode));
	if (!file) {
        perror("Memory allocation failed");
        exit(1);
    }
	file->nr = nr;
	file->prev = file->next = NULL;
	return file;
}

Node *create_node(char *word, FileNode *file) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        perror("Memory allocation failed");
        exit(1);
    }
    node->word = strdup(word);
    if (!node->word) {
        perror("Memory allocation failed");
        free(node);
        exit(1);
    }
	node->files = file;
    node->left = node->right = NULL;
    return node;
}

FileNode *insert_file(FileNode **node, FileNode *file){

	FileNode *filetoinsert = file;
	while(filetoinsert){
		FileNode *new = create_file_node(filetoinsert->nr);
		if (!(*node)){
			(*node) = new;
		}
		else{
			FileNode *current = *node;
			if (filetoinsert->nr < current->nr){
				new->next = (*node);
				(*node)->prev = new;
				(*node) = new;
			}
			else{
				while (current->next != NULL && current->next->nr < filetoinsert->nr)
					current = current->next;

				if (current->next && current->next->nr == filetoinsert->nr)
					return (*node);

				new->next = current->next;
                new->prev = current;

                if (current->next) {
                    current->next->prev = new;
                }
                current->next = new;
			}
		}
		filetoinsert = filetoinsert->next;
		
	}

	return (*node);
}

Node *insert_alphabetically(Node **root, char *word, FileNode *file, int map) {
	Node *nou = create_node(word, file);
	nou->nr_files = 0;

	FileNode *curr = file;
	while (curr){
		nou->nr_files ++;
		curr = curr->next;
	}

    if (!*root){
		(*root) = nou;
		return (*root);
	}

	Node *current = (*root);
	Node *prev = NULL;

	while (current && strcmp(word, current->word) > 0){
		prev = current;
		current = current->right;
	}
	if (current && strcmp(word, current->word) == 0){
		if (map){
			return current;
		}
		insert_file(&current->files, file);
		FileNode *curr = current->files;
		int nr = 0;
		while (curr){
			nr ++;
			curr = curr->next;
		}
		current->nr_files = nr;
		return current;
	}

	if (!current) { 
        prev->right = nou;
        nou->left = prev;
    } else if (!prev) {  
        nou->right = *root;
        (*root)->left = nou;
        *root = nou;
    } else {
        prev->right = nou;
        nou->left = prev;
        nou->right = current;
        current->left = nou;
    }
	
	return nou;
}

Node *insert(Node **root, char *word, FileNode *file, int map) {
	Node *nou = create_node(word, file);
	nou->nr_files = 0;
	FileNode *curr = file;
	while (curr){
		nou->nr_files ++;
		curr = curr->next;
	}

    if (!*root){
		(*root) = nou;
		return (*root);
	}

	Node *current = (*root);

	while (current->right != NULL && strcmp(word, current->word)){
			current = current->right;
		}
	if (strcmp(word, current->word) == 0){
		if (map){
			return current;
		}
		insert_file(&current->files, file);
		FileNode *curr = current->files;
		int nr = 0;
		while (curr){
			nr ++;
			curr = curr->next;
		}
		current->nr_files = nr;
		return current;
	}
	
	nou->left = current;
	current->right = nou;
	
	return nou;
}

Node *insert_sorted(Node **root, char *word, FileNode *files) {
    Node *new_node = create_node(word, files);
    FileNode *curr = files;
    new_node->nr_files = 0;
    while (curr) {  
        new_node->nr_files++;
        curr = curr->next;
    }

    if (!(*root)) {
        *root = new_node;
        return *root;
    }

    Node *current = *root;

    while (current != NULL) {
        if (new_node->nr_files > current->nr_files) {
            new_node->right = current;
            new_node->left = current->left;
            if (current->left) {
                current->left->right = new_node;
            } else {
                *root = new_node; 
            }
            current->left = new_node;
            return new_node;
        } else if (new_node->nr_files == current->nr_files && strcmp(word, current->word) < 0) {
            new_node->right = current;
            new_node->left = current->left;
            if (current->left) {
                current->left->right = new_node;
            } else {
                *root = new_node; 
            }
            current->left = new_node;
            return new_node;
        }
        if (current->right == NULL) {
            break; 
        }
        current = current->right;
    }

    current->right = new_node;
    new_node->left = current;
    return new_node;
}

void *reduce(void *arg){
    ReduceArg *arguments = (ReduceArg *) arg;
	
	char filename[15];
	sprintf(filename, "%c%s", arguments->letter, ".txt");
	FILE *file = fopen(filename, "w+");
	if (!file) {
        perror("Error opening file");
        exit(1);
    }

	Node *root = NULL;
	
	Node **lists = arguments->maps;
	for (int i = 1; i <= arguments->nr_fisiere; i++){
		Node *current = lists[i];
		while (current != NULL){
			if (current->word[0] == arguments->letter){
				insert_alphabetically(&root, current->word, current->files, 0);
			}
			current = current->right;
		}
	}

	Node *root_sorted = NULL;
	Node *current = root;
	while (current){
		insert_sorted(&root_sorted, current->word, current->files);
		current = current->right;
	}

	current = root_sorted;
	while (current){
		fprintf(file, "%s:[", current->word);
		int nr = 0;
		FileNode *curr = current->files;
		while (curr){
			nr++;
			fprintf(file, "%d", curr->nr);
			if (nr < current->nr_files)
				fprintf(file, " ");
			curr = curr->next;
			
		}
		fprintf(file, "]\n");
		current = current->right;
	}
	fclose(file);
    return NULL;
}
os_task_t *create_task(void (*action)(void *), void *arg, void (*destroy_arg)(void *))
{
	os_task_t *t;

	t = malloc(sizeof(*t));
	if (t == NULL){
		perror("malloc");
		exit(1);
	}

	t->action = action;		
	t->argument = arg;		
	t->destroy_arg = destroy_arg;	

	return t;
}

void destroy_task(os_task_t *t)
{
	if (t->destroy_arg != NULL)
		t->destroy_arg(t->argument);
	free(t);
}

void enqueue_task(os_threadpool_t *tp, os_task_t *t)
{
	assert(tp != NULL);
	assert(t != NULL);

	queue_list *new = &(t->list);

	pthread_mutex_lock(&(tp->mutex));
	list_add_tail(&tp->head, new);
	pthread_mutex_unlock(&(tp->mutex));

	if (tp->start == 0) {
		tp->start = 1;
		pthread_cond_broadcast(&tp->cond_start_map);
	} else {
		pthread_cond_signal(&(tp->condition));
	}
}

static int queue_is_empty(os_threadpool_t *tp)
{
	return list_empty(&tp->head);
}

os_task_t *dequeue_task(os_threadpool_t *tp)
{
	os_task_t *t;

	pthread_mutex_lock(&tp->mutex);
	while (queue_is_empty(tp)) {
		tp->waiting_threads++;
		
		if (tp->waiting_threads == tp->num_threads_M) {
			pthread_cond_broadcast(&tp->condition);
			pthread_mutex_unlock(&tp->mutex);
			return NULL;
		}
		pthread_cond_wait(&tp->condition, &tp->mutex);
		if (tp->waiting_threads == tp->num_threads_M) {
			pthread_mutex_unlock(&tp->mutex);
			return NULL;
		}
		tp->waiting_threads--;
	}

	t = list_entry(tp->head.next, os_task_t, list);

	list_del(tp->head.next);
	pthread_mutex_unlock(&tp->mutex);

	return t;
}
os_task_t *dequeue_task2(os_threadpool_t *tp)
{
	os_task_t *t;

	pthread_mutex_lock(&tp->mutex);
	while (queue_is_empty(tp)) {
		tp->waiting_threads++;
		
		if (tp->waiting_threads == tp->num_threads_R + tp->num_threads_M) {
			pthread_cond_broadcast(&tp->condition);
			pthread_mutex_unlock(&tp->mutex);
			return NULL;
		}
		pthread_cond_wait(&tp->condition, &tp->mutex);
		if (tp->waiting_threads == tp->num_threads_R + tp->num_threads_M) {
			pthread_mutex_unlock(&tp->mutex);
			return NULL;
		}
		tp->waiting_threads--;
	}

	t = list_entry(tp->head.next, os_task_t, list);
	list_del(tp->head.next);
	pthread_mutex_unlock(&tp->mutex);

	return t;
}

void *functie(void *arg){
	os_threadpool_t *tp = (os_threadpool_t *) arg;
	pthread_mutex_lock(&tp->mutex_start);
	if (tp->finished_threads < tp->num_threads_M){
		pthread_cond_wait(&tp->cond_start_reduce, &tp->mutex_start);
	}
	pthread_mutex_unlock(&tp->mutex_start);

	while (1) {
		os_task_t *t;
		t = dequeue_task2(tp);
		if (t == NULL){
			break;
		}
		t->action(t->argument);
		destroy_task(t);
	}	
	return NULL;
}

static void *thread_loop_function(void *arg)
{
	os_threadpool_t *tp = (os_threadpool_t *) arg;
	pthread_mutex_lock(&tp->mutex);

	if (tp->start == 0)
		pthread_cond_wait(&tp->cond_start_map, &tp->mutex);

	pthread_mutex_unlock(&tp->mutex);

	while (1) {
		os_task_t *t;

		t = dequeue_task(tp);
		
		if (t == NULL){
			pthread_mutex_lock(&tp->mutex_start);
			
			if (tp->finished_threads == tp->num_threads_M - 1){
				for (char c = 'a'; c <= 'z'; c++){
					ReduceArg *arguments = malloc(sizeof(ReduceArg));
					arguments->letter = c;
					arguments->maps = tp->list_of_lists;
					arguments->nr_fisiere = tp->nr_fisiere;
					os_task_t *new_task = create_task((void *)reduce, (void *)arguments, NULL);
					enqueue_task(tp, new_task);
				}
			}
			tp->finished_threads ++;
			pthread_mutex_unlock(&tp->mutex_start);
			pthread_barrier_wait((&tp->bariera));
			pthread_cond_broadcast(&tp->cond_start_reduce);
			
			
			break;
		}
		t->action(t->argument);

		ThreadArgs *args = (ThreadArgs *)t->argument;
		
		destroy_task(t);
		tp->list_of_lists[args->id] = args->map;
	}

	return NULL;
}

void wait_for_completion(os_threadpool_t *tp)
{
	for (unsigned int i = 1; i <= tp->num_threads_M + tp->num_threads_R; i++)
		pthread_join(tp->threads[i], NULL);
		
}

os_threadpool_t *create_threadpool(unsigned int num_threads_M, unsigned int num_threads_R, int nr_fisiere)
{
	os_threadpool_t *tp = NULL;
	int rc;

	tp = malloc(sizeof(*tp));
	if (tp == NULL){
		perror("malloc");
		exit(1);
	}

	tp->start = 0;
	tp->waiting_threads = 0;
	tp->finished_threads = 0;

	list_init(&tp->head);

	rc = pthread_mutex_init(&tp->mutex, NULL);
	if (rc != 0){
		perror("pthread_mutex_init");
		exit(1);
	}
	rc = pthread_mutex_init(&tp->mutex_access_map, NULL);
	if (rc != 0){
		perror("pthread_mutex_init");
		exit(1);
	}
	rc = pthread_mutex_init(&tp->mutex_start, NULL);
	if (rc != 0){
		perror("pthread_mutex_init");
		exit(1);
	}

	pthread_cond_init(&tp->condition, NULL);
	pthread_cond_init(&tp->cond_start_map, NULL);
	pthread_cond_init(&tp->cond_start_reduce, NULL);
	pthread_barrier_init(&tp->bariera, NULL, num_threads_M);

	tp->num_threads_M = num_threads_M;
	tp->num_threads_R = num_threads_R;
	tp->nr_fisiere = nr_fisiere;
	//tp->list_of_lists = malloc(26 * sizeof(Node *));

	tp->list_of_lists = malloc((nr_fisiere + 1) * sizeof(Node *));
	for (int i = 1; i <= nr_fisiere; i++)
		tp->list_of_lists[i] = malloc(sizeof(Node));
	
	tp->threads = malloc((tp->num_threads_M + tp->num_threads_R + 1) * sizeof(*tp->threads));
	if (tp->threads == NULL){
		perror("malloc");
		exit(1);
	}
	for (int id = 1; id <= num_threads_M + num_threads_R; id++) {
		if (id <= num_threads_M){
			rc = pthread_create(&tp->threads[id], NULL, &thread_loop_function, (void *) tp);
			if (rc < 0){
				perror("pthread_create");
				exit(1);
			}
		}
		else{
			rc = pthread_create(&tp->threads[id], NULL, &functie, (void *) tp);
			if (rc < 0){
				perror("pthread_create");
				exit(1);
			}
		}    
		
    }
	return tp;
}

void destroy_threadpool(os_threadpool_t *tp)
{
	queue_list *n, *p;

	pthread_mutex_destroy(&tp->mutex);
	pthread_mutex_destroy(&tp->mutex_access_map);
	pthread_mutex_destroy(&tp->mutex_start);

	pthread_cond_destroy(&tp->cond_start_map);
	pthread_cond_destroy(&tp->condition);
	pthread_cond_destroy(&tp->cond_start_reduce);

	pthread_barrier_destroy(&tp->bariera);
	list_for_each_safe(n, p, &tp->head) {
		list_del(n);
		destroy_task(list_entry(n, os_task_t, list));
	}

	free(tp->threads);
	free(tp);
}
