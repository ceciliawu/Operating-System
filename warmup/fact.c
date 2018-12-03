#include "common.h"
//factorial function
int factorial(int fact_number)
{
    if (fact_number == 0)
        return 1;
    else
        return fact_number * factorial(fact_number-1);
}
int
main( int argc, char *argv[] )
{
    //printf("number of argc is %d\n",argc);
    int int_num = atoi(argv[1]);
    float float_num = atof(argv[1]);
    //printf("temp num is %d,\n", int_num);
    if ((argc == 1) || (int_num <= 0) || (int_num != float_num))
        printf("Huh?\n");
    else
    {   
        if( int_num > 12)
            printf("Overflow\n");
        else
        {
            int number = factorial(int_num);
            printf("%d\n",number);
        }
    }
    return 0;
}

