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
        pa->array=realloc(pa->array,sizeof(position) * pa->max_size);
    }
    pa->array[pa->size] = p;
    pa->size++;
}


inline void remove_from_position_array(position_array * pa, unsigned int index)
{
    if(index >= pa->size) return;
    pa->array[index] = pa->array[pa->size-1];
    pa->size--;
}

unsigned int number_of_lightbulb_possible(lu_puzzle *p, unsigned int line, unsigned int column, position * lb_pos)
{
    int l = 0, c = 0, width = p-> width, height = p->height;
    unsigned int count = 0;
    unsigned char current_status = 0;
    position lb_pos_local = {0,0};

    for(l = line + 1; l < height ; l++)
    {
        current_status = p->data[l * width + column];
        if(current_status <= lusq_block_any) break;
        if(current_status == lusq_empty)
        {
            count++;
            lb_pos_local = (position){l,column};
        }
    }
    for(l = line - 1; l >= 0 ; l--)
    {
        current_status = p->data[l * width + column];
        if(current_status <= lusq_block_any) break;
        if(current_status == lusq_empty)
        {
            count++;
            lb_pos_local = (position){l,column};
        }
    }

    for(c = column + 1; l < width ; c++)
    {
        current_status = p->data[line * width + c];
        if(current_status <= lusq_block_any) break;
        if(current_status == lusq_empty)
        {
            count++;
            lb_pos_local = (position){line,c};
        }
    }
    for(c = column - 1; l >= 0 ; c--)
    {
        current_status = p->data[line * width + c];
        if(current_status <= lusq_block_any) break;
        if(current_status == lusq_empty)
        {
            count++;
            lb_pos_local = (position){line,c};
        }
    }

    *lb_pos = lb_pos_local;
    return count;
}

char is_alone(lu_puzzle *p, unsigned int line, unsigned int column)
{
    int l = 0, c = 0, width = p-> width, height = p->height;
    unsigned int count = 0;
    unsigned char current_status = 0;

    for(l = line + 1; l < height ; l++)
    {
        current_status = p->data[l * width + column];
        if(current_status == lusq_block_any) break;
        count+= current_status == lusq_empty;
    }
    for(l = line - 1; l >= 0 ; l--)
    {
        current_status = p->data[l * width + column];
        if(current_status <= lusq_block_any) break;
        count+= current_status == lusq_empty;
    }

    for(c = column + 1; c < width ; c++)
    {
        current_status = p->data[line * width + c];
        if(current_status <= lusq_block_any) break;
        count+= current_status == lusq_empty;
    }
    for(c = column - 1; c >= 0 ; c--)
    {
        current_status = p->data[line * width + c];
        if(current_status <= lusq_block_any) break;
        count+= current_status == lusq_empty;
    }

    return count == 0;
}


int_array new_int_array()
{
    return (int_array){malloc(sizeof(int)*SIZE), 0, SIZE};
}

int_array new_int_array_with_size(unsigned int size)
{
    if(size == 0) size = SIZE;
    return (int_array){malloc(sizeof(int)*size), 0, size};
}

void delete_int_array(int_array * ia)
{
    free(ia->array);
    ia->size = 0;
    ia->max_size = 0;
}

void add_to_int_array(int_array * ia, int p)
{
    if(ia->size == ia->max_size)
    {
        ia->max_size = (ia->max_size <= 0) ? SIZE : ia->max_size * 2;
        ia->array=realloc(ia->array, sizeof(int) * ia->max_size);
    }
    ia->array[ia->size] = p;
    ia->size++;
}
