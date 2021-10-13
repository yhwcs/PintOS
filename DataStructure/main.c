#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "hash.h"
#include "bitmap.h"

#define FALSE 0
#define TRUE 1
#define MAX_INPUT_SIZE 256
#define TYPE_LIST 0
#define TYPE_HASH 1
#define TYPE_BITMAP 2

typedef struct _list_item{
	struct list_elem elem;
	int data;
}list_item;

typedef struct _hash_item{
	struct hash_elem elem;
	int data;
}hash_item;

struct list LIST[10];
struct hash HASH[10];
struct bitmap* BITMAP[10];

struct hash_iterator iterator;

int struct_type;
int list_idx;
int hash_idx;
int bitmap_idx;


int read_command(char** command_token, int token_cnt);
char** tokenize_command(char* command, int* token_cnt);
bool list_less(const struct list_elem* a, const struct list_elem* b, void* aux);
struct list_elem* find_elem(int list_idx, int item_idx);

unsigned hash_hash(const struct hash_elem* a, void* aux);
bool hash_less(const struct hash_elem* a, const struct hash_elem* b, void* aux);
void hash_print(struct hash_elem* a, void* aux);
void hash_square(struct hash_elem* a, void* aux);
void hash_triple(struct hash_elem* a, void* aux);

int main(){
	int result=1;
	char command[MAX_INPUT_SIZE];
	char** command_token;
	int token_cnt=0;
	while(result){
		fgets(command,sizeof(command),stdin);
		command_token=tokenize_command(command, &token_cnt);
		result=read_command(command_token, token_cnt);
	}
	return 0;
}

struct list_elem* find_elem(int list_idx, int item_idx){
	struct list_elem* temp;
	temp = list_begin(&LIST[list_idx]);
	for(int i=0; i<item_idx; i++)
		temp = list_next(temp);
	return temp;
}

bool list_less(const struct list_elem* a, const struct list_elem* b, void* aux){
	list_item* a_item;
	a_item = list_entry(a, list_item, elem);
	list_item* b_item;
	b_item = list_entry(b, list_item, elem);
	if(a_item->data < b_item->data)
		return true;
	else
		return false;
}

unsigned hash_hash(const struct hash_elem* a, void* aux){
	hash_item* temp;
	temp = hash_entry(a, hash_item, elem);
	return hash_int(temp->data);
	//return hash_int_2(temp->data);
}

bool hash_less(const struct hash_elem* a, const struct hash_elem* b, void* aux){
	hash_item* a_item;
	a_item = hash_entry(a, hash_item, elem);
	hash_item* b_item;
	b_item = hash_entry(b, hash_item, elem);
	if(a_item->data < b_item->data)
		return true;
	else
		return false;
}

void destructor(struct hash_elem* a, void* aux){
	hash_item* temp;
	temp = hash_entry(a, hash_item, elem);
	free(temp);
}

void hash_print(struct hash_elem* a, void* aux){
	hash_item* temp;
	temp = hash_entry(a, hash_item, elem);
	printf("%d ", temp->data);
}

void hash_square(struct hash_elem* a, void* aux){
	hash_item* temp;
	temp = hash_entry(a, hash_item, elem);
	temp->data = (temp->data) * (temp->data);
}

void hash_triple(struct hash_elem* a, void* aux){
	hash_item* temp;
	temp = hash_entry(a, hash_item, elem);
	temp->data = (temp->data) * (temp->data) * (temp->data);
}

int read_command(char** command_token, int token_cnt){
	if(!strcmp(command_token[0], "quit"))
		return 0;
	else if(!strcmp(command_token[0], "create")){
		if(!strcmp(command_token[1], "list")){
			list_idx=(int)(command_token[2][4]-'0');
			struct_type=TYPE_LIST;
			list_init(&LIST[list_idx]);
		}
		else if(!strcmp(command_token[1], "hashtable")){
			hash_idx=(int)(command_token[2][4]-'0');
			struct_type=TYPE_HASH;
			hash_init(&HASH[hash_idx], hash_hash, hash_less, NULL);
		}
		else if(!strcmp(command_token[1], "bitmap")){
			bitmap_idx=(int)(command_token[2][2]-'0');
			struct_type=TYPE_BITMAP;
			unsigned int size;
			size=atoi(command_token[3]);
			BITMAP[bitmap_idx] = bitmap_create(size);
		}
	}
	else if(!strcmp(command_token[0], "delete")){
		if(struct_type==TYPE_LIST){
			list_idx=(int)(command_token[1][4]-'0');
			struct list_elem* cur;
			while(!list_empty(&LIST[list_idx])){
				cur = list_pop_front(&LIST[list_idx]);
				free(cur);
			}
		}
		else if(struct_type==TYPE_HASH){
			hash_idx=(int)(command_token[1][4]-'0');
			hash_destroy(&HASH[hash_idx], destructor);	
		}
		else if(struct_type==TYPE_BITMAP){
			bitmap_idx=(int)(command_token[1][2]-'0');
			bitmap_destroy(BITMAP[bitmap_idx]);
		}
	}
	else if(!strcmp(command_token[0], "dumpdata")){
		if(struct_type==TYPE_LIST){
			list_idx=(int)(command_token[1][4]-'0');
			if(!list_empty(&LIST[list_idx])){
				struct list_elem* temp;
				temp = list_begin(&LIST[list_idx]);
				list_item* temp_item;
				while(temp != list_end(&LIST[list_idx])){
					temp_item = list_entry(temp, list_item, elem);
					printf("%d ",temp_item->data);
					temp = list_next(temp);
				}
				printf("\n");
				//temp_item = list_entry(temp, list_item, elem);
				//printf("%d\n",temp_item->data);
			}
		}
		else if(struct_type==TYPE_HASH){	
			hash_idx=(int)(command_token[1][4]-'0');
			if(!hash_empty(&HASH[hash_idx])){
				/*
				struct hash_iterator temp;
				hash_first(&iterator, &HASH[hash_idx]);
				hash_item* temp_item;
				while(hash_next(&temp) != NULL){
					temp_item = hash_entry(temp.elem, hash_item, elem);
					printf("%d ", temp_item->data);
				}
				*/
				hash_apply(&HASH[hash_idx], hash_print);
				printf("\n");
			}
		}
		else if(struct_type==TYPE_BITMAP){
			bitmap_idx=(int)(command_token[1][2]-'0');
			if(BITMAP[bitmap_idx]){
				int size;
				size = bitmap_size(BITMAP[bitmap_idx]);
				for(int i=0; i<size; i++){
					if(bitmap_test(BITMAP[bitmap_idx], i) == 1)
						printf("1");
					else
						printf("0");
				}
				printf("\n");
			}
		}
	}
	else if(!strcmp(command_token[0], "list_insert")){
		int item_data, item_idx;	
		list_idx=(int)(command_token[1][4]-'0');
		item_idx=atoi(command_token[2]);
		item_data=atoi(command_token[3]);

		struct list_elem* new;
	   	new = (struct list_elem*)malloc(sizeof(struct list_elem));
		list_item* new_item;
		new_item = list_entry(new, list_item, elem);
		new_item->data = item_data;

		if(item_idx==0 || list_empty(&LIST[list_idx])){
			list_push_front(&LIST[list_idx], new);
		}
		else if(list_size(&LIST[list_idx]) <= item_idx){
			list_push_back(&LIST[list_idx],new);
		}
		else{
			struct list_elem* temp;
			temp = find_elem(list_idx, item_idx);
			list_insert(temp, new);
		}
	}
	else if(!strcmp(command_token[0], "list_insert_ordered")){
		list_idx=(int)(command_token[1][4]-'0');
		int item_data;
		item_data=atoi(command_token[2]);
		
		struct list_elem* new;
		new = (struct list_elem*)malloc(sizeof(struct list_elem));
		list_item* new_item;
		new_item = list_entry(new, list_item, elem);
		new_item->data = item_data;
		list_insert_ordered(&LIST[list_idx],new,list_less,NULL);
	}
	else if(!strcmp(command_token[0], "list_push_front")){
		list_idx=(int)(command_token[1][4]-'0');
		int item_data;
		item_data=atoi(command_token[2]);
		
		struct list_elem* new;
		new = (struct list_elem*)malloc(sizeof(struct list_elem));
		list_item* new_item;
		new_item = list_entry(new, list_item, elem);
		new_item->data = item_data;
		list_push_front(&LIST[list_idx],new);
	}
	else if(!strcmp(command_token[0], "list_push_back")){
		list_idx=(int)(command_token[1][4]-'0');
		int item_data;
		item_data=atoi(command_token[2]);
		
		struct list_elem* new;
		new = (struct list_elem*)malloc(sizeof(struct list_elem));
		list_item* new_item;
		new_item = list_entry(new, list_item, elem);
		new_item->data = item_data;
		list_push_back(&LIST[list_idx],new);
	}
	else if(!strcmp(command_token[0], "list_pop_front")){
		list_idx=(int)(command_token[1][4]-'0');
		list_pop_front(&LIST[list_idx]);
	}
	else if(!strcmp(command_token[0], "list_pop_back")){
		list_idx=(int)(command_token[1][4]-'0');
		list_pop_back(&LIST[list_idx]);
	}
	else if(!strcmp(command_token[0], "list_front")){
		list_idx=(int)(command_token[1][4]-'0');
		
		struct list_elem* temp;
		temp = list_front(&LIST[list_idx]);
		list_item* temp_item;
		temp_item = list_entry(temp, list_item, elem);
		if(temp_item)
			printf("%d\n", temp_item->data);

	}
	else if(!strcmp(command_token[0], "list_back")){
		list_idx=(int)(command_token[1][4]-'0');
		
		struct list_elem* temp;
		temp = list_back(&LIST[list_idx]);
		list_item* temp_item;
		temp_item = list_entry(temp, list_item, elem);
		if(temp_item)
			printf("%d\n", temp_item->data);
	}
	else if(!strcmp(command_token[0], "list_empty")){
		list_idx=(int)(command_token[1][4]-'0');
		if(list_empty(&LIST[list_idx]))
			printf("true\n");
		else
			printf("false\n");
	}
	else if(!strcmp(command_token[0], "list_size")){
		list_idx=(int)(command_token[1][4]-'0');
		printf("%zu\n", (list_size(&LIST[list_idx])));
	}
	else if(!strcmp(command_token[0], "list_max")){
		list_idx=(int)(command_token[1][4]-'0');
		struct list_elem* temp;
		temp = list_max(&LIST[list_idx], list_less, NULL);
		list_item* temp_item;
		temp_item = list_entry(temp, list_item, elem);
		printf("%d\n", temp_item->data);
	}
	else if(!strcmp(command_token[0], "list_min")){
		list_idx=(int)(command_token[1][4]-'0');
		struct list_elem* temp;
		temp = list_min(&LIST[list_idx], list_less, NULL);
		list_item* temp_item;
		temp_item = list_entry(temp, list_item, elem);
		printf("%d\n", temp_item->data);
	}
	else if(!strcmp(command_token[0], "list_remove")){
		list_idx=(int)(command_token[1][4]-'0');
		int item_idx;
		item_idx=atoi(command_token[2]);
		struct list_elem* temp;
		temp = find_elem(list_idx, item_idx);
		list_remove(temp);
	}
	else if(!strcmp(command_token[0], "list_reverse")){
		list_idx=(int)(command_token[1][4]-'0');
		list_reverse(&LIST[list_idx]);
	}
	else if(!strcmp(command_token[0], "list_sort")){
		list_idx=(int)(command_token[1][4]-'0');
		list_sort(&LIST[list_idx], list_less, NULL);
	}
	else if(!strcmp(command_token[0], "list_splice")){
		list_idx=(int)(command_token[1][4]-'0');
		int insert_list_idx;
		insert_list_idx=(int)(command_token[3][4]-'0');
		int before, first, last;
		before = atoi(command_token[2]);
		first = atoi(command_token[4]);
		last = atoi(command_token[5]);

		struct list_elem *before_elem, *first_elem, *last_elem;
		before_elem = find_elem(list_idx, before);
		first_elem = find_elem(insert_list_idx, first);
		last_elem = find_elem(insert_list_idx, last);
		list_splice(before_elem, first_elem, last_elem);

	}
	else if(!strcmp(command_token[0], "list_unique")){
		list_idx=(int)(command_token[1][4]-'0');
		if(token_cnt == 2)
			list_unique(&LIST[list_idx], NULL, list_less, NULL);
		else{
			int duplicate_list_idx;
			duplicate_list_idx=(int)(command_token[2][4]-'0');
			list_unique(&LIST[list_idx], &LIST[duplicate_list_idx], list_less, NULL);
		}
	}
	else if(!strcmp(command_token[0], "list_swap")){
		list_idx=(int)(command_token[1][4]-'0');
		int a_idx, b_idx;
		a_idx = atoi(command_token[2]);
		b_idx = atoi(command_token[3]);
		
		struct list_elem *a_elem, *b_elem;
		a_elem = find_elem(list_idx, a_idx);
		b_elem = find_elem(list_idx, b_idx);
		list_swap(a_elem, b_elem);
	}
	else if(!strcmp(command_token[0], "list_shuffle")){
		list_idx=(int)(command_token[1][4]-'0');
		list_shuffle(&LIST[list_idx]);
	}
	//HASH
	else if(!strcmp(command_token[0], "hash_insert")){
		hash_idx=(int)(command_token[1][4]-'0');
		int item_data;
		item_data = atoi(command_token[2]);
		
		struct hash_elem* new;
		new = (struct hash_elem*)malloc(sizeof(struct hash_elem));
		hash_item* new_item;
		new_item = hash_entry(new, hash_item, elem);
		new_item->data = item_data;
		hash_insert(&HASH[hash_idx], new);
	}
	else if(!strcmp(command_token[0], "hash_replace")){
		hash_idx=(int)(command_token[1][4]-'0');
		int item_data;
		item_data = atoi(command_token[2]);

		struct hash_elem* temp;
		temp = (struct hash_elem*)malloc(sizeof(struct hash_elem));
		hash_item* temp_item;
		temp_item = hash_entry(temp, hash_item, elem);
		temp_item->data = item_data;
		hash_replace(&HASH[hash_idx], temp);
	}
	else if(!strcmp(command_token[0], "hash_find")){
		hash_idx=(int)(command_token[1][4]-'0');
		int item_data;
		item_data = atoi(command_token[2]);

		struct hash_elem* temp;
		temp = (struct hash_elem*)malloc(sizeof(struct hash_elem));
		hash_item* temp_item;
		temp_item = hash_entry(temp, hash_item, elem);
		temp_item->data = item_data;

		struct hash_elem* find;
		find = hash_find(&HASH[hash_idx], temp);
		if(find != NULL){
			printf("%d\n", hash_entry(find, hash_item, elem)->data);
		}
	}
	else if(!strcmp(command_token[0], "hash_empty")){
		hash_idx=(int)(command_token[1][4]-'0');
		if(hash_empty(&HASH[hash_idx]))
			printf("true\n");
		else
			printf("false\n");
	}
	else if(!strcmp(command_token[0], "hash_size")){
		hash_idx=(int)(command_token[1][4]-'0');
		printf("%zu\n", hash_size(&HASH[hash_idx]));
	}
	else if(!strcmp(command_token[0], "hash_clear")){
		hash_idx=(int)(command_token[1][4]-'0');
		hash_clear(&HASH[hash_idx], destructor);
	}
	else if(!strcmp(command_token[0], "hash_delete")){	
		hash_idx=(int)(command_token[1][4]-'0');
		int item_data;
		item_data = atoi(command_token[2]);
		
		struct hash_elem* temp;
		temp = (struct hash_elem*)malloc(sizeof(struct hash_elem));
		hash_item* temp_item;
		temp_item = hash_entry(temp, hash_item, elem);
		temp_item->data = item_data;
		hash_delete(&HASH[hash_idx], temp);
	}
	else if(!strcmp(command_token[0], "hash_apply")){
		hash_idx=(int)(command_token[1][4]-'0');
		if(!strcmp(command_token[2], "square"))
			hash_apply(&HASH[hash_idx], hash_square);
		else if(!strcmp(command_token[2], "triple"))
			hash_apply(&HASH[hash_idx], hash_triple);
	}
	//BITMAP
	else if(!strcmp(command_token[0], "bitmap_test")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int bit_idx;
		bit_idx = atoi(command_token[2]);
		if(bitmap_test(BITMAP[bitmap_idx], bit_idx) == 1)
			printf("true\n");
		else
			printf("false\n");
	}
	else if(!strcmp(command_token[0], "bitmap_size")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		size_t size;
		size = bitmap_size(BITMAP[bitmap_idx]);
		printf("%zu\n", size);
	}
	else if(!strcmp(command_token[0], "bitmap_set")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		bool value;
		unsigned int bit_idx;
		bit_idx = atoi(command_token[2]);
		if(!strcmp(command_token[3], "true"))
			value=true;
		else
			value=false;
		bitmap_set(BITMAP[bitmap_idx], bit_idx, value);
	}
	else if(!strcmp(command_token[0], "bitmap_set_all")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		bool value;
		if(!strcmp(command_token[2], "true"))
			value=true;
		else
			value=false;
		bitmap_set_all(BITMAP[bitmap_idx], value);
	}
	else if(!strcmp(command_token[0], "bitmap_set_multiple")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int start, count;
		bool value;
		start = atoi(command_token[2]);
		count = atoi(command_token[3]);
		if(!strcmp(command_token[4], "true"))
			value=true;
		else
			value=false;
		bitmap_set_multiple(BITMAP[bitmap_idx], start, count, value);
	}
	else if(!strcmp(command_token[0], "bitmap_scan")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int start, count;
		size_t start_idx;
		bool value;
		start = atoi(command_token[2]);
		count = atoi(command_token[3]);
		if(!strcmp(command_token[4], "true"))
			value=true;
		else
			value=false;
		start_idx = bitmap_scan(BITMAP[bitmap_idx], start, count, value);
		printf("%zu\n", start_idx);	
	}
	else if(!strcmp(command_token[0], "bitmap_scan_and_flip")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int start, count;
		size_t start_idx;
		bool value;
		start = atoi(command_token[2]);
		count = atoi(command_token[3]);
		if(!strcmp(command_token[4], "true"))
			value=true;
		else
			value=false;
		start_idx = bitmap_scan_and_flip(BITMAP[bitmap_idx], start, count, value);
		printf("%zu\n", start_idx);
	}
	else if(!strcmp(command_token[0], "bitmap_reset")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int bit_idx;
		bit_idx = atoi(command_token[2]);
		bitmap_reset(BITMAP[bitmap_idx], bit_idx);
	}
	else if(!strcmp(command_token[0], "bitmap_none")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int start, count;
		start = atoi(command_token[2]);
		count = atoi(command_token[3]);
		if(bitmap_none(BITMAP[bitmap_idx], start, count))
			printf("true\n");
		else
			printf("false\n");
	}
	else if(!strcmp(command_token[0], "bitmap_flip")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int bit_idx;
		bit_idx = atoi(command_token[2]);
		bitmap_flip(BITMAP[bitmap_idx], bit_idx);
	}
	else if(!strcmp(command_token[0], "bitmap_dump")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		bitmap_dump(BITMAP[bitmap_idx]);
	}
	else if(!strcmp(command_token[0], "bitmap_mark")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int bit_idx;
		bit_idx = atoi(command_token[2]);
		bitmap_mark(BITMAP[bitmap_idx], bit_idx);
	}
	else if(!strcmp(command_token[0], "bitmap_count")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int start, count;
		bool value;
		start = atoi(command_token[2]);
		count = atoi(command_token[3]);
		if(!strcmp(command_token[4], "true"))
			value=true;
		else
			value=false;
		
		size_t value_cnt;
		value_cnt = bitmap_count(BITMAP[bitmap_idx], start, count, value);
		printf("%zu\n", value_cnt);
	}
	else if(!strcmp(command_token[0], "bitmap_contains")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int start, count;
		bool value;
		start = atoi(command_token[2]);
		count = atoi(command_token[3]);
		if(!strcmp(command_token[4], "true"))
			value=true;
		else
			value=false;
		if(bitmap_contains(BITMAP[bitmap_idx], start, count, value))
			printf("true\n");
		else
			printf("false\n");
	}
	else if(!strcmp(command_token[0], "bitmap_all")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int start, count;
		start = atoi(command_token[2]);
		count = atoi(command_token[3]);
		if(bitmap_all(BITMAP[bitmap_idx], start, count))
			printf("true\n");
		else
			printf("false\n");
	}
	else if(!strcmp(command_token[0], "bitmap_any")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int start, count;
		start = atoi(command_token[2]);
		count = atoi(command_token[3]);
		if(bitmap_any(BITMAP[bitmap_idx], start, count))
			printf("true\n");
		else
			printf("false\n");
	}
	else if(!strcmp(command_token[0], "bitmap_expand")){
		bitmap_idx=(int)(command_token[1][2]-'0');
		unsigned int size;
		size = atoi(command_token[2]);
		BITMAP[bitmap_idx] = bitmap_expand(BITMAP[bitmap_idx], size);
	}
	return 1;
}

char** tokenize_command(char* command, int* token_cnt){
	char temp_command[(int)strlen(command)];
	char* temp;
	char* sep=" \n\r\t";
	int count=0;
	strcpy(temp_command,command);
	temp=strtok(temp_command, sep);
	while(1){
		count++;
		temp=strtok(NULL, sep);
		if(temp==NULL)
			break;
	}
	*token_cnt=count;
	char** command_token;
	command_token=(char**)malloc(sizeof(char*)*count);
	for(int i=0; i<count; i++)
		command_token[i]=(char*)malloc(sizeof(char)*(MAX_INPUT_SIZE));
	strcpy(temp_command,command);
	temp=strtok(temp_command, sep);
	strcpy(command_token[0], temp);
	for(int j=1; j<count; j++){
		temp=strtok(NULL, sep);
		strcpy(command_token[j], temp);	
	}
	return command_token;
}
