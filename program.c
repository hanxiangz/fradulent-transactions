/* Fraudulent credit cards:
 *
 * program code written by Hanxiang Zhang, student number 1080496, June 2020
 *
 */

#include<stdio.h> 
#include<stdlib.h> 
#include<string.h>
#include<assert.h> 

#define CARD_ID_LEN 8 
#define TRANS_ID_LEN 12
/* I made a decision to set a maximum to the number of transactions for 
each credit card, since this would probably be the case in real life */
#define MAX_NUM_TRANS 100 
#define STAGEHEADER "=========================Stage %d========================="
#define STAGE_NUM_ONE 1 
#define STAGE_NUM_TWO 2
#define STAGE_NUM_THREE 3
#define STAGE_NUM_FOUR 4

/* -------------------------typedefs---------------------------- */ 

typedef struct {
    int year, month, day; 
} date_t; 

typedef struct {
    int hour, minute, second; 
} time_t; 

typedef struct {
    char id[TRANS_ID_LEN+1]; 
    char card_id[CARD_ID_LEN+1]; 
    date_t date; 
    time_t time; 
    int amount;  
} transaction_t; 

typedef struct node node_t; 

struct node {
    transaction_t data; 
    node_t *next; 
}; 

typedef struct {
    node_t *head;
    node_t *foot; 
    
} list_t; 

typedef struct { 
    char id[CARD_ID_LEN+1];
    int daily_lim, trans_lim; 
    date_t transaction_dates[MAX_NUM_TRANS]; 
    int transaction_amount[MAX_NUM_TRANS]; 
    int num_trans; 
} record_t;

/* ------------------------my functions------------------------- */ 

void print_stage_header(int stage_num);
int read_a_record(record_t* one_record[], int *num_records);  
int same_date(date_t d1, date_t d2);
void print_fraud_or_not(node_t *temp_node, record_t *record[], 
                        int index, int n); 
void stage_one(record_t* record[], int *num_records); 
void stage_two(record_t* record[], int *num_records, int *max_elems);
void stage_three(list_t *trans_list);
void stage_four(list_t *trans_list, record_t *record[], int *num_records);

/* ------------external functions, unimelb comp10002------------ */ 
// the funcions below are derived from lecture 07, slide 26

list_t *insert_at_foot(list_t *list, transaction_t info); 
list_t *make_empty_list(void);
int is_empty_list(list_t *list); 
transaction_t get_head(list_t *list); 
list_t *get_tail(list_t *list); 
void free_list(list_t *list); 

// the function below is derived from lecture 05, slide 27 

int binary_search(record_t* record[], int lo, int hi, 
                  char trans_card_id[], int *locn); 

/* ------------------------------------------------------------- */ 

int main(int argc, char *argv[]) { 
    int max_elems = 10; 
    record_t* *record = (record_t**)malloc(max_elems * sizeof(record_t*));
    assert(record != NULL); 
    
    int i; 
    // create space for each structure being pointed to
    for (i = 0; i < max_elems; i++) {
        record[i] = (record_t*)malloc(sizeof(record_t)); 
        assert(record); 
    } 
    
    int num_records = 0; 
    stage_one(record, &num_records); 
    stage_two(record, &num_records, &max_elems); 
    
    // create an empty linked list for stage 3
    list_t *trans_list; 
    trans_list = make_empty_list(); 
    stage_three(trans_list); 
    
    // initialise the number of transactions in each credit card record to be 0
    for (i = 0; i < num_records; i++) { 
        record[i]->num_trans = 0; 
    } 
    stage_four(trans_list, record, &num_records); 
    
    // free the linked list 
    transaction_t head; 
    while (!is_empty_list(trans_list)) {
        head = get_head(trans_list); 
        trans_list = get_tail(trans_list); 
    }
    free_list(trans_list); 
    trans_list = NULL; 
    
    //free the credit card record array 
    for (i = 0; i < num_records; i++) {
        free(record[i]); 
        record[i] = NULL;
    }
    free(record); 
    record = NULL; 
    
    return 0; 
} 

/* -----------------------listops functions--------------------- */

/* inserts a new item (structure data_cpy), onto the foot of the linked list */
list_t 
*insert_at_foot(list_t *list, transaction_t data_cpy) {
    node_t *new;
	new = (node_t*)malloc(sizeof(*new));
	assert(list!=NULL && new!=NULL);
	new->data = data_cpy;
	new->next = NULL;
	if (list->foot==NULL) {
		// this is the first insertion into the list 
		list->head = list->foot = new;
	} else {
		list->foot->next = new;
		list->foot = new;
	}
	return list;
} 

/* creates an empty linked list */ 
list_t
*make_empty_list(void) {
	list_t *list;
	list = (list_t*)malloc(sizeof(*list));
	assert(list!=NULL);
	list->head = list->foot = NULL;
	return list;
} 

/* checks whether the linked list is empty */ 
int
is_empty_list(list_t *list) {
	assert(list!=NULL);
	return list->head==NULL;
}

/* retrieves the first transaction stored in the linked list */ 
transaction_t
get_head(list_t *list) {
	assert(list!=NULL && list->head!=NULL);
	return list->head->data;
}

/* delete current head of the list, update the new head to be next item */ 
list_t
*get_tail(list_t *list) {
	node_t *oldhead;
	assert(list!=NULL && list->head!=NULL);
	oldhead = list->head;
	list->head = list->head->next;
	if (list->head==NULL) {
		/* the only list node just got deleted */
		list->foot = NULL;
	}
	free(oldhead);
	return list;
}

/* free memory stored in a list */ 
void
free_list(list_t *list) {
	node_t *curr, *prev;
	assert(list!=NULL);
	curr = list->head;
	while (curr) {
		prev = curr;
		curr = curr->next;
		free(prev);
	}
	free(list);
}

/* ------------------------------------------------------------- */

/* print a stage header */
void 
print_stage_header(int stage_num) {
	printf(STAGEHEADER, stage_num);
}

/* reads a credit card record, storing its information in an array */ 
int 
read_a_record(record_t* record[], int *num_records) { 
    char * end = "%%%%%%%%%%"; 
    
    // beware that we may have just stored %%% in record
    scanf("%s", record[*num_records]->id); 
    
    // if we have just read %%%... then exit function
    if (strcmp(record[*num_records]->id, end) == 0) {
        return 0; 
    }
    
    if (scanf("%d %d", &record[*num_records]->daily_lim, 
    &record[*num_records]->trans_lim) == 2) {
        return 1;
    }
    return 0; 
}

/* performs binary search, returning the index location of the search key */
int
binary_search(record_t* record[], int lo, int hi, 
              char trans_card_id[], int *locn) {
    int mid, outcome;
    
    mid = (lo+hi)/2;
    if ((outcome = strcmp(trans_card_id, record[mid]->id)) < 0) {
        return binary_search(record, lo, mid, trans_card_id, locn);
    } else if (outcome > 0) {
        return binary_search(record, mid+1, hi, 
                             trans_card_id, locn);
    } else {
        *locn = mid;
        return *locn; 
    }
}

/* returns 1 if the two days are the same date */
int 
same_date(date_t d1, date_t d2) { 
    return (d1.year == d2.year && d1.month == d2.month && d1.day == d2.day); 
}

/* prints whether transaction amount exceeds daily and transaction limit */ 
void 
print_fraud_or_not(node_t *temp_node, record_t *record[], int index, int n) {
    int i, over_trans, daily_total, sameday; 

    // check to see if the amount we just stored is over transaction limit
    if (temp_node->data.amount > record[index]->trans_lim) {
        over_trans = 1; 
    } else {
        over_trans = 0; 
    }
        
    // check to see if over daily limit 
    if (n == 1) { // we only have 1 transaction stored so far
        if (over_trans == 1 && 
            temp_node->data.amount > record[index]->daily_lim) {
            printf("%s             OVER_BOTH_LIMITS\n", temp_node->data.id);
        } else if (over_trans ==1 && 
            temp_node->data.amount < record[index]->daily_lim) {
            printf("%s             OVER_TRANS_LIMIT\n", temp_node->data.id);
        } else {
            printf("%s             IN_BOTH_LIMITS\n", temp_node->data.id);
        } 
    } else { // >1 transaction stored = possibility of exceeding daily_lim 
        daily_total = temp_node->data.amount; 
        for (i = 0; i < n - 1; i++) { 
            sameday = same_date(temp_node->data.date, 
            record[index]->transaction_dates[i]); 
            if (sameday) {  
                daily_total += record[index]->transaction_amount[i]; 
            }
        } 
        if (daily_total > record[index]->daily_lim) { // exceed daily_lim
            if (over_trans == 0) {
                printf("%s             OVER_DAILY_LIMIT\n", temp_node->data.id); 
            } else {
                printf("%s             OVER_BOTH_LIMITS\n", temp_node->data.id); 
            } 
        } else { // under daily_lim
            if (over_trans == 0) {
                printf("%s             IN_BOTH_LIMITS\n", temp_node->data.id); 
            } else {
                printf("%s             OVER_TRANS_LIMIT\n", temp_node->data.id); 
            } 
        } 
    } 
    
}


/* read one credit card record onto the record array */ 
void 
stage_one(record_t* record[], int *num_records) {
    print_stage_header(STAGE_NUM_ONE);
    printf("\n"); 

    read_a_record(record, num_records); 
    *num_records += 1;  
    
    printf("Card ID: %s\n", record[0]->id); 
    printf("Daily limit: %d\n", record[0]->daily_lim); 
    printf("Transaction limit: %d\n", record[0]->trans_lim); 
    printf("\n"); 
} 

/* read all credit records in the input, then store them onto records array */ 
void 
stage_two(record_t* record[], int *num_records, int *max_elems) {
    int new_arr_size = *max_elems, old_arr_size;
    
    print_stage_header(STAGE_NUM_TWO);
    printf("\n"); 
     
    while (read_a_record(record, num_records) == 1) { 
        *num_records += 1; // corresponds to how many credit card records read
        
        // update the array size if it has reached full capacity
        if (*num_records == new_arr_size) {
            old_arr_size = new_arr_size; 
            new_arr_size *= 2;  
            record = realloc(record, new_arr_size * sizeof(record_t*)); 
            assert(record); 
            
            // create new memory space for a structure, one at a time 
            record[*num_records] = (record_t*)malloc(sizeof(record_t));
            assert(record[*num_records]);
            
        } else if (*num_records > old_arr_size) { 
            record[*num_records] = (record_t*)malloc(sizeof(record_t));
            assert(record[*num_records]);
        } 
        
    } 
    
    int i;
    double total = 0.0; // calculate total of daily limits in card record
    for (i = 0; i < *num_records; i++) {
        total += record[i]->daily_lim; 
    } 
    
    int max_translim = record[0]->trans_lim; 
    int index = 0; 
    // find the position of the card with the largest transaction limit
    for (i = 0; i < *num_records; i++) {
        if (record[i]->trans_lim > max_translim) {
            max_translim = record[i]->trans_lim;
            index = i; 
        } 
    } 
    
    printf("Number of credit cards: %d\n", *num_records); 
    printf("Average daily limit: %.2f\n", total/(*num_records)); 
    printf("Card with the largest transaction limit: %s\n", record[index]->id);
    printf("\n"); 
} 

/* read transactions in the input, then store them in a linked list */ 
void 
stage_three(list_t *trans_list) {

    print_stage_header(STAGE_NUM_THREE);
    printf("\n"); 

    // create temporary data structure to store scanned items 
    transaction_t temp;
    
    // read the whole transaction 
    while (scanf("%s %s %d:%d:%d:%d:%d:%d %d", temp.id, temp.card_id, 
                 &temp.date.year, &temp.date.month, &temp.date.day, 
                 &temp.time.hour, &temp.time.minute, &temp.time.second, 
                 &temp.amount) == 9) {
        trans_list = insert_at_foot(trans_list, temp); 
    }  
    
    // iterate through linked list, printing out transaction ids
    node_t *temp_node; 
    temp_node = trans_list->head; 
    while (temp_node != NULL) {
        printf("%s\n", temp_node->data.id); 
        temp_node = temp_node->next; 
    }
    printf("\n"); 
} 

/* checks for fraudulent transactions */ 
void 
stage_four(list_t *trans_list, record_t *record[], int *num_records) {
    print_stage_header(STAGE_NUM_FOUR);
    printf("\n");
    
    // create a temporary array to hold a card id 
    char *trans_card_id = (char*)malloc((CARD_ID_LEN + 1) * sizeof(char)); 
    assert(trans_card_id); 
    
    // iterate through linked list with the help of a temporary node
    int n, index, lo = 0, hi = *num_records - 1, locn;
    node_t *temp_node = trans_list->head; 
    while (temp_node != NULL) {
        trans_card_id = temp_node->data.card_id;
        // binary search returns the location of card id in record array
        index = binary_search(record, lo, hi, trans_card_id, &locn); 
        
        // store transaction amount & date in record, then update num_trans
        n = record[index]->num_trans; 
        record[index]->transaction_amount[n] = temp_node->data.amount;
        record[index]->transaction_dates[n] = temp_node->data.date; 
        record[index]->num_trans += 1; 
        n = record[index]->num_trans;
        
        print_fraud_or_not(temp_node, record, index, n); 
        temp_node = temp_node->next; 
    }
     
} 

/* average time complexity of the above algorithm is nlogm since a binary 
search (logm time complexity) is performed for every credit card search,
in each transaction (n) */ 


