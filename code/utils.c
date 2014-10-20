#include "utils.h"

position_array new_position_array()
{
    return (position_array){malloc(sizeof(position)*SIZE), 0, SIZE};
}

position_array new_position_array_with_size(unsigned int size)
{
    if(size == 0) size = SIZE;
    return (position_array){malloc(sizeof(position)*size), 0, size};
}

void delete_position_array(position_array * pa)
{
    free(pa->array);
    pa->size = 0;
    pa->max_size = 0;
}


void add_to_position_array(position_array * pa, position p)
{
    if(pa->size == pa->max_size)
    {
        pa->max_size = (pa->max_size <= 0) ? SIZE : pa->max_size * 2;
        pa->array=realloc(pa->array,pa->max_size);
    }
    pa->array[pa->size++] = p;
}


inline void remove_from_position_array(position_array * pa, unsigned int index)
{
    if(index >= pa->size) return;
    pa->array[index] = pa->array[pa->size-1];
    pa->size--;
}



