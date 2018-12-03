#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "point.h"
#include "sorted_points.h"

struct sorted_points {
	/* you can define this struct to have whatever fields you want. */
    double x;
    double y;
    double distance;
    struct sorted_points *next;
};
// function to calculate the distance
double calculate_distance(double x, double y)
{
    double distance = sqrt(x*x + y*y);
    return distance;
}

struct sorted_points *
sp_init()
{
	struct sorted_points *sp;

	sp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
	assert(sp);
        sp->x = 0;
        sp->y = 0;
	sp->next = NULL;
        sp->distance = 0;
	return sp;
}

void
sp_destroy(struct sorted_points *sp)
{
    struct sorted_points * current = sp->next;
    while(current!=NULL)
    {   
        struct sorted_points * temp = current;
        current = current->next;
        free(temp);
    }
	free(sp);
}

int
sp_add_point(struct sorted_points *sp, double x, double y)
{   //if the list is empty, change the value of the already existing one
    if (sp->next == NULL)
    {
        struct sorted_points * add = sp_init();
        add->x = x;
        add->y = y;
        add->distance = calculate_distance(x,y);
        sp->next = add;
        
        //printf("first element values are x: %G, d: %G\n",sp->x,sp->distance);
        return 1;
    }
    // if the list already exists
    else
    {   //initiate another sorted points to add in
        struct sorted_points * head;
        head = sp->next;
        struct sorted_points * add = sp_init();
        add->x = x;
        add->y = y;
        add->distance = calculate_distance(x,y);
        //printf("add distance is %G\n",add->distance);
        // iterate through the entire list and find the right spot
        struct sorted_points * current = head;
        struct sorted_points * previous = NULL;
        while(current!=NULL)
        {   //if the distance now is greater, insert before
            if ((current->distance) > (add->distance))
            {   //if inserted at the beginning
                if(current == head)
                {   //printf("beginning\n");
                    sp->next = add;
                    add->next = current;
                }
                else{
                previous->next = add;
                add->next = current;
                }
                return 1;
            }
            //handle the condition when the two distances are equal
            if ((current->distance) == (add->distance))
            {   //if current-x < add-x, append after current
                if((current->x) < (add->x))
                {
                    add->next = current->next;
                    current->next = add;    
                }
                //if current-x > add-x,append before current
                else if((current->x) > (add->x))
                {
                    if (current == head)
                    {
                        sp->next = add;
                        add->next = current;
                    }
                    else
                    {
                    previous->next = add;
                    add->next = current;
                    }
                }
                //same x coordinates, compare the y coordinates
                else
                {//if current-y < add-y, append after
                    //printf("x equal\n");
                    if((current->y) < (add->y))
                    {
                        add->next = current->next;
                        current->next = add;    
                    }
                 //else either current-y greater or equal to add-y, append before
                    else
                    {
                       if (current == head)
                        {
                        sp->next = add;
                        add->next = current;
                        }
                       else
                       {
                        previous->next = add;
                        add->next = current;
                       }
                    }                   
                }
                return 1;
            }
            previous = current;
            current = current->next;
        }
        //if need to append at the end
        previous->next = add;
        return 1;
    }
	return 0;
}

int
sp_remove_first(struct sorted_points *sp, struct point *ret)
{
    struct sorted_points * head = sp->next;
    if(head == NULL)
        return 0;
    ret = point_set(ret,head->x,head->y);
    struct sorted_points * temp = head;
    sp->next = head->next;
    free(temp);
    return 1;
}

int
sp_remove_last(struct sorted_points *sp, struct point *ret)
{
    struct sorted_points * head = sp->next;
    if(head==NULL)
        return 0;
    struct sorted_points * current = head;
    struct sorted_points * previous = NULL;
    while(current->next != NULL)
    {
        previous = current;
        current = current->next;
    }
    ret = point_set(ret,current->x,current->y);
    free(current);
    if(previous == NULL)
    {
        sp->next=NULL;
    }
    else
    {
        previous->next=NULL;
    }
	return 1;
}

int
sp_remove_by_index(struct sorted_points *sp, int index, struct point *ret)
{   int num = 0;
    if(sp->next == NULL)
        return 0;
    struct sorted_points * head = sp->next;
    struct sorted_points * current = head;
    struct sorted_points * previous = NULL;
    while((num<=index)&&(current!=NULL))
    {  
        if (num==index)
        {
            //remove the element
            if(index == 0)
            {
                sp_remove_first(sp,ret);
            }
            else if (current->next == NULL)
            {
                sp_remove_last(sp,ret);
            }
            else
            {
                previous->next = current->next;
                ret = point_set(ret,current->x,current->y);
                free(current);
            }
            
            return 1;
        }
        previous = current;
        current = current->next;
        num++;
    }
	return 0;
}

int
sp_delete_duplicates(struct sorted_points *sp)
{
    int num = 0;
    if(sp->next == NULL)
        return 0;
    struct sorted_points * head = sp->next;
    struct sorted_points * current = head;
    struct sorted_points * previous = NULL;    
    previous = current;
    current = current->next;
    while (current!= NULL)
    {
        if((previous->x == current->x)&&(previous->y == current->y))
        {
            struct sorted_points * temp = current;
            previous->next = current->next;
            current = current->next;
            free(temp);
            num++;
        }
        else
        {
            previous = current;
            current = current->next;
        }
    }
    
    return num;
}

// temporary test program
/*int main(void)
{   struct sorted_points *sp;
    sp = sp_init();
    sp_add_point(sp, 7,8);
    sp_add_point(sp,11,11);
    sp_add_point(sp, 9,10);
    sp_add_point(sp,15,16);
    sp_add_point(sp,14,0);
    sp_add_point(sp,0,14);
    sp_add_point(sp,7,8);
    struct sorted_points * current = sp->next;
    printf("returned!\n");
    while(current!=NULL)
    {
        printf("x is %G, y is %G, distance is %G\n",current->x,current->y,current->distance);
        current = current->next;
    }
    return 1;
}
*/