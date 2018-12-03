#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include "common.h"
#include "wc.h"
long hash_size;
struct wc {
	/* you can define this struct to have whatever fields you want. */
    char value[100];
    bool empty;
    int num; 
    struct wc *next;    
};

// hash function borrowed from open source
int hash_function(long size, char *key ) {

	unsigned long int hashval;
	int i = 0;

	/* Convert our string to an integer */
	while( hashval < ULONG_MAX && i < strlen( key ) ) {
		hashval = hashval << 8;
		hashval += key[ i ];
		i++;
	}

	return hashval % size;
}

struct wc *
wc_init(char *word_array, long size)
{
	struct wc *wc;
        if (size > 50000000)
            hash_size = 10000000;
        else
            hash_size = size/5;          
	wc = (struct wc *)malloc(sizeof(struct wc) * (hash_size));
	assert(wc);
        int i;
        
        for(i=0; i< (hash_size);i++)
        {
            wc[i].empty=true;
            wc[i].next = NULL;
        }
          
        //parse the array into words and store them into the hash table
        i=0;
        int first_char = 0;
        int num = 0;
        while (i<size)
        {
            while (!isspace(word_array[i]))
            {
                i++;
                num++;
            }
            if(num != 0)
            {
                char *temp;
                temp = (char *)malloc(sizeof(char) * (num+1));
                int j;
                for(j=0;j<num;j++)
                {
                    temp [j] = word_array[first_char];
                    first_char++;
                }     
                temp[j] = '\0';
                //put it into the hash table if this node is empty
                int key = hash_function(hash_size,temp);
                if(wc[key].empty)
                {
                    wc[key].num=1;
                    strcpy(wc[key].value,temp);
                    wc[key].empty = false;
                }
                else if(strcmp(wc[key].value,temp) == 0)
                {
                    wc[key].num++;
                }
                //handles collision
                else
                {   
                    // printf("collision!\n");
                    bool not_exist = true;
                    if(wc[key].next != NULL)
                    {
                        struct wc * current = wc[key].next;
                        while(current->next!=NULL)
                        {
                            if(strcmp(current->value,temp) == 0)
                            {
                                current->num ++;
                                not_exist = false;
                                break;
                            }
                            else
                            current = current->next;
                        }
                    
                        if(not_exist)
                        {
                             if(strcmp(current->value,temp) == 0)
                            {
                                current->num ++;
                            }
                             else
                             {
                            struct wc *temp_wc;
                            temp_wc = (struct wc *)malloc(sizeof(struct wc));
                            temp_wc->empty = false;
                            temp_wc->next = NULL;
                            temp_wc->num = 1;
                            strcpy(temp_wc->value,temp);
                            current->next = temp_wc;
                             }
                        }
                    }
                    else
                    {
                        //printf("add to linked list\n");
                        struct wc *temp_wc;
                        temp_wc = (struct wc *)malloc(sizeof(struct wc));
                        temp_wc->empty = false;
                        temp_wc->next = NULL;
                        temp_wc->num = 1;
                        strcpy(temp_wc->value,temp);
                        wc[key].next = temp_wc;
                    }
                }
                free(temp);

            }
            num = 0;
            i++;
            first_char = i;
        } 
        
        
	return wc;
}

void
wc_output(struct wc *wc)
{
        int l = 0;
        while(l<hash_size)
        {
            if (!wc[l].empty)
                printf("%s:%d\n",wc[l].value,wc[l].num);
            if(wc[l].next!=NULL)
            {
                struct wc * curr = wc[l].next;
                while(curr != NULL)
                {
                    printf("%s:%d\n",curr->value,curr->num);
                    curr = curr->next;
                }
            }
              l++;  
        }
}

void
wc_destroy(struct wc *wc)
{
     int l = 0;
        while(l<hash_size)
        {
            if (wc[l].next != NULL)
            {  
                    struct wc * curr = wc[l].next;
                    while(curr != NULL)
                    {
                        struct wc * temp_d = curr;
                        curr = curr->next;
                        free(temp_d);
                    }
            }
        l++;  
            
        }
    free(wc);
}

