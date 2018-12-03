#include <assert.h>
#include "common.h"
#include "point.h"
#include <math.h>

void
point_translate(struct point *p, double x, double y)
{   double old_x = point_X(p);
    double old_y = point_Y(p);
    double new_x = old_x + x;
    double new_y = old_y + y;
    point_set(p, new_x, new_y);
}

double
point_distance(const struct point *p1, const struct point *p2)
{
    double point1_x = point_X(p1);
    double point1_y = point_Y(p1);
    double point2_x = point_X(p2);
    double point2_y = point_Y(p2);
    double result = sqrt (pow((point2_x - point1_x),2.0) + pow((point2_y-point1_y),2.0));
    return result;
}

int
point_compare(const struct point *p1, const struct point *p2)
{
    double point1_x = point_X(p1);
    double point1_y = point_Y(p1);
    double point2_x = point_X(p2);
    double point2_y = point_Y(p2);
    
    //calculate and compare the two distances
    double distance_1 = sqrt(pow(point1_x,2.0) + pow(point1_y,2.0));
    double distance_2 = sqrt(pow(point2_x,2.0) + pow(point2_y,2.0));
    
    if (distance_1 < distance_2)
	return -1;
    if (distance_1 == distance_2)
        return 0;
    return 1;
}
