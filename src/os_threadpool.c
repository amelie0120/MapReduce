// SPDX-License-Identifier: BSD-3-Clause

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "os_threadpool.h"
//#include "log/log.h"
//#include "utils.h"

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
	//FileNode *f = create_file_node(file);
	node->files = file;
    node->left = node->right = NULL;
    return node;
}

FileNode *insert_last(FileNode **node, FileNode *file){

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

Node *insert(Node **root, char *word, FileNode *file, int reduce) {
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
		insert_last(&current->files, file);
		return current;
	}

	
	
	nou->left = current;
	current->right = nou;
	
	return nou;
}

Node *insert_final(Node **root, char *word, FileNode *files){
	Node *nou = create_node(word, files);
	FileNode *curr = files;
	nou->nr_files = 0;
	while (curr){
		nou->nr_files++;
		curr = curr->next;
	}

	if (!(*root)){
		(*root) = nou;
		return (*root);
	}

	nou->right = (*root);
	(*root)->left = nou;
	(*root) = nou;
	return (*root);
}

// Node *insert_sorted(Node **root, char *word, FileNode *files, int nrfiles){
// 	// printf("%s to be inserted\n", word);
	
// 	Node *nou = create_node(word, files);
// 	FileNode *curr = files;
// 	nou->nr_files = 0;
// 	while (curr){
// 		nou->nr_files++;
// 		curr = curr->next;
// 	}
// 	if (!*root){
// 		(*root) = nou;
		
// 		return (*root);
// 	}
// 	Node *current = (*root);

// 	while (current->right != NULL && current->nr_files > nou->nr_files){
// 		current = current->right;
// 	}
// 	while (current->right != NULL && strcmp(word, current->word) > 0 && current->nr_files == nou->nr_files){
// 		current = current->right;
// 	}

// 	if (current->right != NULL){
// 		if (strcmp(word, current->word) < 0 && nou->nr_files >= current->nr_files){
// 			//adaug inainte
			
// 			if (current == (*root)){
// 				nou->right = (*root);
// 				(*root)->left = nou;
// 				(*root) = nou;
// 			}
// 			else{
// 				nou->right = current;
// 				nou->left = current->left;
// 				current->left->right = nou;
// 				current->left = nou;
// 			}
// 		}
// 		else{
// 			//adaug dupa; nu e ult elem sigur
// 			nou->left = current;
// 			nou->right = current->right;
// 			current->right->left = nou;
// 			current->right = nou;
			

// 		}
// 	}
// 	else if (current->right == NULL){
// 		if (strcmp(word, current->word) < 0 && nou->nr_files >= current->nr_files){
// 			//Node *new = create_node(word, files);
// 			//new->nr_files = 1;
// 			if (current == (*root)){
// 				nou->right = (*root);
// 				(*root)->left = nou;
// 				(*root) = nou;
// 			}
// 			else{
// 				nou->right = current;
// 				nou->left = current->left;
// 				current->left->right = nou;
// 				current->left = nou;
// 			}
			
// 		}
// 		else {
// 			//Node *new = create_node(word, files);
// 			//new->nr_files = 1;
// 			nou->left = current;
// 			current->right = nou;
// 		}
// 	}
// 	return nou;

// }

Node *insert_sorted(Node **root, char *word, FileNode *files, int nrfiles) {
    // Create the new node
    Node *new_node = create_node(word, files);
    FileNode *curr = files;
    new_node->nr_files = 0;
    while (curr) {  // Count the number of files
        new_node->nr_files++;
        curr = curr->next;
    }

    // If the list is empty, set root to the new node
    if (!(*root)) {
        *root = new_node;
        return *root;
    }

    Node *current = *root;

    // Traverse the list to find the correct position
    while (current != NULL) {
        if (new_node->nr_files > current->nr_files) {
            // Insert before the current node
            new_node->right = current;
            new_node->left = current->left;
            if (current->left) {
                current->left->right = new_node;
            } else {
                *root = new_node; // Update root if inserting at the head
            }
            current->left = new_node;
            return new_node;
        } else if (new_node->nr_files == current->nr_files && strcmp(word, current->word) < 0) {
            // Insert before the current node (lexicographical order)
            new_node->right = current;
            new_node->left = current->left;
            if (current->left) {
                current->left->right = new_node;
            } else {
                *root = new_node; // Update root if inserting at the head
            }
            current->left = new_node;
            return new_node;
        }
        // Move to the next node
        if (current->right == NULL) {
            break; // Stop if we're at the end
        }
        current = current->right;
    }

    // Insert at the end of the list
    current->right = new_node;
    new_node->left = current;
    return new_node;
}


void swap_nodes(Node **head, char *word1, char *word2){
	Node *prev1 = NULL, *prev2 = NULL;
    Node *curr1 = *head, *curr2 = *head;

    // Find x and its previous node
    while (curr1 && strcmp(curr1->word, word1)) {
        prev1 = curr1;
        curr1 = curr1->right;
    }

    // Find y and its previous node
    while (curr2 && strcmp(curr2->word, word2)) {
        prev2 = curr2;
        curr2 = curr2->right;
    }

    // If either x or y is not present, return
    if (!curr1 || !curr2) return;

    // If x is not the head, update prevX->next
    if (prev1)
        prev1->right = curr2;
    else // Else make y the new head
        *head = curr2;

    // If y is not the head, update prevY->next
    if (prev2)
        prev2->right = curr1;
    else // Else make x the new head
        *head = curr1;

    // Swap next pointers
    Node *temp = curr1->right;
    curr1->right = curr2->right;
    curr2->right = temp;
}

void sort_list(Node **root){
	if (*root == NULL || (*root)->right == NULL)
		return;

	//int go = 1;
	Node *curr1 = (*root);
	while (curr1){
		//go = 0;
		Node *curr2 = curr1->right;
		while (curr2){
			// if (curr1 == curr2)
			// 	curr2 = curr2->right;
			// 	continue;
			if (curr1->nr_files == curr2->nr_files){
				if (strcmp(curr1->word, curr2->word) > 0){
					//printf("o sa faca swap intre %s si %s\n", curr1->word, curr2->word);
					// char *word = curr1->word;
					// //int files = curr1->
					// curr1->word = curr2->word;
					// curr2->word = word;
					swap_nodes(root, curr1->word, curr2->word);
					Node *aux = curr1;
					curr1 = curr2;
					curr2 = aux;
					//printf("acum curr1 e %s si curr2 e %s\n", curr1->word, curr2->word);
					//go = 1;
				}
			}
			else if (curr1->nr_files < curr2->nr_files){
				//printf("o sa faca swap intre %s si %s\n", curr1->word, curr2->word);
				swap_nodes(root, curr1->word, curr2->word);
				Node *aux = curr1;
				curr1 = curr2;
				curr2 = aux;
				//printf("acum curr1 e %s si curr2 e %s\n", curr1->word, curr2->word);
				//go = 1;
			}
			curr2 = curr2->right;
			//else if (curr1->nr_files < curr2->nr_files)
		}
		curr1 = curr1->right;
	}
}

void *reduce(void *arg){
    ReduceArg *arguments = (ReduceArg *) arg;
	//printf("litera e %c\n", arguments->letter);
	
	char filename[15];
	//printf("test_par/%c%s\n", arguments->letter, ".txt");
	sprintf(filename, "%c%s", arguments->letter, ".txt");
	//printf("in fisierul %s\n", filename);
	FILE *file = fopen(filename, "w+");
	if (!file) {
        perror("Error opening file");
        exit(1);
    }
	//printf("in fisierul %s\n", filename);

	Node *root = NULL;
	//int nr_nodes = 0;
	
	Node *current = arguments->map_big;
	while (current != NULL){
		if (current->word[0] == arguments->letter){
			insert_sorted(&root, current->word, current->files, 0);
		}
		current = current->right;
	}

	// current = root;
	// while (current){
	// 	printf("%s:", current->word);
	// 	//int nr = 0;
	// 	FileNode *curr = current->files;
	// 	while (curr){
	// 		printf("%d ", curr->nr);
	// 		curr = curr->next;
	// 		//nr++;
	// 	}
	// 	//current->nr_files = nr;
	// 	printf("are %d fisiere\n", current->nr_files);
	// 	current = current->right;
	// }

	//sort_list(&root);
	current = root;
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
		//current->nr_files = nr;
		//printf("are %d fisiere\n", current->nr_files);
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

	os_list_node_t *new = &(t->list);

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
	while(1){
		if (tp->finished_threads == tp->num_threads_M){
			break;
		}
		
	}

	while (1) {
		os_task_t *t;
		t = dequeue_task2(tp);
		if (t == NULL){
			break;
		}
		t->action(t->argument);
		destroy_task(t);
	}	
	
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
				// Node *current = tp->map_big;
				// while (current){
				// 	printf("%s\n", current->word);
				// 	current = current->right;
				// }
				for (char c = 'a'; c <= 'z'; c++){
					ReduceArg *arguments = malloc(sizeof(ReduceArg));
					arguments->letter = c;
					arguments->map_big = tp->map_big;
					os_task_t *new_task = create_task((void *)reduce, (void *)arguments, NULL);
					enqueue_task(tp, new_task);
				}
				// Node *current = tp->map_big;
				// while (current){
				// 	printf("%s:", current->word);
				// 	//int nr = 0;
				// 	FileNode *curr = current->files;
				// 	while (curr){
				// 		printf("%d ", curr->nr);
				// 		curr = curr->next;
				// 		//nr++;
				// 	}
				// 	printf("\n");
				// 	//current->nr_files = nr;
				// 	//printf("are %d fisiere\n", current->nr_files);
				// 	current = current->right;
				// }
			}
			
			tp->finished_threads ++;
			pthread_mutex_unlock(&tp->mutex_start);
			
			
			break;
		}
		t->action(t->argument);

		ThreadArgs *args = (ThreadArgs *)t->argument;
		
		destroy_task(t);

		Node *current = args->map;
		while(current != NULL){
			//Node *nou = create_node(current->word, args->id);

			pthread_mutex_lock(&tp->mutex_access_map);
			FileNode *nodfisier = create_file_node(args->id);
			insert(&tp->map_big, current->word, nodfisier, 0);
			// Node *current_big = tp->map_big;
			// tp->number_pairs ++;
			// if (!current_big){
			// 	tp->map_big = nou;
			// }
			// else{
			// 	while (current_big->right != NULL){
			// 		current_big = current_big->right;
			// 	}
			// 	nou->left = current_big;
			// 	current_big->right = nou;
			// }
			pthread_mutex_unlock(&tp->mutex_access_map);

			current = current->right;
		}
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
	tp->semafor = 0;
	tp->number_pairs = 0;
	//tp->find_letter = 'a';
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

	tp->num_threads_M = num_threads_M;
	tp->num_threads_R = num_threads_R;
	//tp->list_of_lists = malloc(26 * sizeof(Node *));
	
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
	os_list_node_t *n, *p;

	pthread_mutex_destroy(&tp->mutex);
	//pthread_mutex_destroy(&tp->mutex_semafor);
	pthread_mutex_destroy(&tp->mutex_access_map);
	pthread_mutex_destroy(&tp->mutex_start);
	pthread_cond_destroy(&tp->cond_start_map);
	pthread_cond_destroy(&tp->condition);
	//pthread_cond_destroy(&tp->cond_start_reduce);
	list_for_each_safe(n, p, &tp->head) {
		list_del(n);
		destroy_task(list_entry(n, os_task_t, list));
	}

	free(tp->threads);
	free(tp);
}
