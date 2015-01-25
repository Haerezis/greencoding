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


void disable_cpu(unsigned int number_cpu_left)
{
    unsigned int i = 0, num_cpu_online = 0;
    FILE * fd = NULL;
    char buffer[255] = {0};

    num_cpu_online = sysconf( _SC_NPROCESSORS_ONLN );
    for(i=0 ; i<number_cpu_left && i<num_cpu_online ; i++)
    {
        sprintf(buffer,"/sys/devices/system/cpu/cpu%u/online",i);
        if((fd = fopen(buffer,"w")) == NULL) continue;
        fputc('1',fd);
        fclose(fd);
    }
    for(i=number_cpu_left ; i<num_cpu_online ; i++)
    {
        sprintf(buffer,"/sys/devices/system/cpu/cpu%u/online",i);
        if((fd = fopen(buffer,"w")) == NULL) continue;
        fputc('0',fd);
        fclose(fd);
    }
}

void enable_all_cpu()
{
    unsigned int i = 0, num_cpu_online = 0;
    FILE * fd = NULL;
    char buffer[255] = {0};

    num_cpu_online = sysconf( _SC_NPROCESSORS_ONLN );
    for(i=0 ; i<num_cpu_online ; i++)
    {
        sprintf(buffer,"/sys/devices/system/cpu/cpu%u/online",i);
        if((fd = fopen(buffer,"w")) == NULL) continue; 
        fputc('1',fd);
        fclose(fd);
    }
}

static int gettid()
{
#ifdef SYS_gettid
    return syscall(SYS_gettid);
#else
#error "SYS_gettid unavailable on this system"
#endif
}

#define CHECK_ERROR(ctx,fct,message) { int result = fct; \
    if (result != DVFS_SUCCESS) { \
        printf(message" (%s).\n",dvfs_strerror(result)); \
        dvfs_stop(ctx); \
        exit(EXIT_FAILURE); \
    }}

void slowdown_cpu(dvfs_ctx * ctx, unsigned int number_cpu_left)
{
    unsigned int i = 0;
    int id = 0;
    unsigned int freq;
    unsigned int nb_core = sysconf( _SC_NPROCESSORS_ONLN);
    cpu_set_t cpu_set;
    const dvfs_core * core;

    CHECK_ERROR(ctx,dvfs_set_gov(ctx, "userspace"),"Unable to set governor");

    for(i = 0; i < number_cpu_left ; i++)
    {
        CHECK_ERROR(ctx,dvfs_get_core(ctx, &core, 0),"Get core");
        CHECK_ERROR(ctx,dvfs_core_set_freq(core , core->freqs[core->nb_freqs-1]),"Set highest freqs");
    }
    for(i = number_cpu_left; i < nb_core ; i++)
    {
        CHECK_ERROR(ctx,dvfs_get_core(ctx, &core, i),"Get core");
        CHECK_ERROR(ctx,dvfs_core_set_freq(core , core->freqs[0]),"Set lowest freqs");
    }

    CHECK_ERROR(ctx,dvfs_get_core(ctx, &core, 0),"Get core");
    CHECK_ERROR(ctx,dvfs_core_get_current_freq(core, &freq), "Get current freq");
    printf("Current frequence : %f MHz\n", (float)freq / 1000.0);


    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);
    id = gettid(); //Maybe use getpid() instead?
    if(sched_setaffinity(id, sizeof(cpu_set_t), &cpu_set) != 0)
    {
        //Error
        perror( "sched_setaffinity" );
        exit(EXIT_FAILURE);
    }
}

void slowdown(dvfs_ctx * ctx)
{
    const dvfs_core * core;    
    CHECK_ERROR(ctx,dvfs_get_core(ctx, &core, 0),"Get core");
    CHECK_ERROR(ctx,dvfs_core_set_freq(core , core->freqs[core->nb_freqs/2]),"Set highest freqs");
}
