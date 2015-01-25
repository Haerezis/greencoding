#include "solve.h"

position_array left_, right_ ,top_ ,bottom_,center_;

void print_solutions(position_array pa_empty, int * solutions)
{
    unsigned int i = 0;
    while(solutions[i] != -2)
    {
        printf("[");
        while(solutions[i] != -1)
        {
            printf(" (%u,%u),", pa_empty.array[solutions[i]].line, pa_empty.array[solutions[i]].column);
            i++;
        }
        printf("]\n");
        i++;
    }
}

void print_solution(position_array pa_empty, int * solutions)
{
    unsigned int i = 0;
    printf("[");
    while(solutions[i] != -1)
    {
        printf(" (%u,%u),", pa_empty.array[solutions[i]].line, pa_empty.array[solutions[i]].column);
        i++;
    }
    printf("]\n");
}



void print_class(position_array* pa_array, unsigned int pa_array_size)
{
    unsigned int index_array, index;
    for(index_array = 0 ; index_array < pa_array_size; index_array++)
    {
        for(index = 0; index < pa_array[index_array].size ; index++)
        {
            printf("(%u,%u) | ",pa_array[index_array].array[index].line ,pa_array[index_array].array[index].column);
        }
        printf(".\n");
    }
}


void classify_positions(lu_puzzle *p, position_array ** pa_array_empty, position_array ** pa_array_impossible, unsigned int * pa_array_size, position_array pa_empty)
{
    position_array * pa_local_array_empty = NULL, *pa_local_array_impossible = NULL;
    unsigned int pa_local_array_size = 0;

    int width = p->width;
    int height = p->height;

    unsigned int index_empty = 0, index = 0;
    int c = 0, l = 0;

    position_array current_pa_empty = {0,0,0};
    position_array current_pa_impossible = {0,0,0};
    unsigned int total_empty_size = 0;
    position_array pa_flood = new_position_array_with_size(pa_empty.size);
    position current_pos, flood_pos;

    pa_local_array_empty = (position_array*) malloc(sizeof(position_array) * pa_empty.size);
    pa_local_array_impossible = (position_array*) malloc(sizeof(position_array) * pa_empty.size);

    for(index_empty = 0; index_empty < pa_empty.size ; index_empty++)
    {
        current_pos = pa_empty.array[index_empty];
        if(p->data[current_pos.line*width + current_pos.column] == lusq_flood || p->data[current_pos.line*width + current_pos.column] == lusq_flood_impossible) continue;//Déjà ajouté à une classe

        current_pa_empty = new_position_array_with_size(pa_empty.size - total_empty_size);//Taille dégressive
        current_pa_impossible = new_position_array();
        add_to_position_array(&pa_flood,current_pos);

        while(pa_flood.size > 0)
        {
            flood_pos = pa_flood.array[pa_flood.size-1];
            pa_flood.size--;

            if(p->data[flood_pos.line*width + flood_pos.column] == lusq_empty)
            {
                p->data[flood_pos.line*width + flood_pos.column] = lusq_flood;
                add_to_position_array(&current_pa_empty, flood_pos);
            }
            else if(p->data[flood_pos.line*width + flood_pos.column] == lusq_impossible)
            {
                p->data[flood_pos.line*width + flood_pos.column] = lusq_flood_impossible;
                add_to_position_array(&current_pa_impossible, flood_pos);
            }
            else continue;
            
            c = flood_pos.column + 1;
            while((c < width) && (p->data[flood_pos.line*width + c] == lusq_enlighted)) c++;
            if((c < width) && ((p->data[flood_pos.line*width + c] == lusq_empty) || (p->data[flood_pos.line*width + c] == lusq_impossible)))
            {
                add_to_position_array(&pa_flood, (position){flood_pos.line,c});
            }

            c = flood_pos.column - 1;
            while((c >= 0) && (p->data[flood_pos.line*width + c] == lusq_enlighted)) c--;
            if((c >= 0) && ((p->data[flood_pos.line*width + c] == lusq_empty) || (p->data[flood_pos.line*width + c] == lusq_impossible)))
            {
                add_to_position_array(&pa_flood, (position){flood_pos.line,c});
            }

            l = flood_pos.line + 1;
            while((l < height) && (p->data[l*width + flood_pos.column] == lusq_enlighted)) l++;
            if((l < height) && ((p->data[l*width + flood_pos.column] == lusq_empty) || (p->data[l*width + flood_pos.column] == lusq_impossible)))
            {
                add_to_position_array(&pa_flood, (position){l,flood_pos.column});
            }

            l = flood_pos.line - 1;
            while((l >= 0) && (p->data[l*width + flood_pos.column] == lusq_enlighted)) l--;
            if((l >= 0) && ((p->data[l*width + flood_pos.column] == lusq_empty) || (p->data[l*width + flood_pos.column] == lusq_impossible)))
            {
                add_to_position_array(&pa_flood, (position){l,flood_pos.column});
            }
        }
        pa_local_array_empty[pa_local_array_size] = current_pa_empty;
        pa_local_array_impossible[pa_local_array_size] = current_pa_impossible;
        pa_local_array_size++;
        total_empty_size += current_pa_empty.size;
    }

    unsigned int i = 0;
    for(i = 0; i< pa_local_array_size; i++)
    {
        for(index = 0; index < pa_local_array_empty[i].size ; index++)
        {
            current_pos = pa_local_array_empty[i].array[index];
            p->data[current_pos.line*width + current_pos.column] = lusq_empty;
        }

        for(index = 0; index < pa_local_array_impossible[i].size ; index++)
        {
            current_pos = pa_local_array_impossible[i].array[index];
            p->data[current_pos.line*width + current_pos.column] = lusq_impossible;
        }
    }

    *pa_array_empty = pa_local_array_empty;
    *pa_array_impossible = pa_local_array_impossible;
    *pa_array_size = pa_local_array_size;

    return;
}

//Regarde s'il est possible d'allumer cette emplacement (si l'emplacement est déjà allumé, ou si il y a conflit avec un mur à coté).
int impossible_to_light(lu_puzzle *p, position pos)
{
    char boolean = 0;
    boolean = (p->data[pos.line * p->width + pos.column] != lusq_empty)
    || wall_saturated(p,pos.column - 1, pos.line)
    || wall_saturated(p,pos.column + 1, pos.line)
    || wall_saturated(p,pos.column, pos.line-1)
    || wall_saturated(p,pos.column, pos.line+1);

    return boolean;
}

int possible_to_light(lu_puzzle *p, position pos)
{
    char boolean = 0;
    boolean = (p->data[pos.line * p->width + pos.column] == lusq_empty)
    && !wall_saturated(p,pos.column - 1, pos.line)
    && !wall_saturated(p,pos.column + 1, pos.line)
    && !wall_saturated(p,pos.column, pos.line-1)
    && !wall_saturated(p,pos.column, pos.line+1);

    return boolean;
}


//Regarde si la solution allume tout les emplacements vide (possiblement impossible) que represente pa_empty
int solution_is_complete(lu_puzzle *p, position_array pa_empty, position_array pa_impossible)
{
    unsigned int index = 0;
    unsigned int return_boolean = 0;

    index = 0;
    while((index < pa_empty.size) &&
          ((p->data[pa_empty.array[index].line*p->width + pa_empty.array[index].column] == lusq_lbulb) ||
           (p->data[pa_empty.array[index].line*p->width + pa_empty.array[index].column] == lusq_enlighted))
         ) index++;
    if(index >= pa_empty.size)
    {
        index = 0;
        while((index < pa_impossible.size) && (p->data[pa_impossible.array[index].line*p->width + pa_impossible.array[index].column] == lusq_enlighted)) index++;
        return_boolean = index >= pa_impossible.size;
    }

    return return_boolean;
}


/*!
 * Résoud un puzzle light-up de façon itérative.
 */
void solve(lu_puzzle *p, position_array pa_empty, position_array pa_impossible, int_array * solutions, unsigned int *sol_id) 
{
    unsigned int index = 0, sol_id_local = 0;
    wh_bufs * whbufs = new_wh_bufs(p->width, p->height, pa_empty.size);
    char *wbuf = NULL, *hbuf = NULL;
    int empty_count = pa_empty.size + pa_impossible.size;

    int_array ia_current_solution = new_int_array();
    *solutions = new_int_array();
    do
    {
        while((index < pa_empty.size) && impossible_to_light(p, pa_empty.array[index])) index++;
        if(index < pa_empty.size)
        {
            pop_wh_buf(whbufs, &wbuf, &hbuf);
            puzzle_light_on_with_bufs(p, pa_empty.array[index].column, pa_empty.array[index].line, wbuf, hbuf, &empty_count);
            add_to_int_array(&ia_current_solution, index);
        }
        else
        {
            //if(solution_is_complete(p, pa_empty, pa_impossible) == 1)
            if(empty_count <= 0)
            {
                if((solutions->size + ia_current_solution.size + 1) > solutions->max_size)
                {
                    solutions->max_size *= 2;
                    solutions->array = (int*)realloc(solutions->array, sizeof(int) * solutions->max_size);
                }
                memcpy(&solutions->array[solutions->size], ia_current_solution.array, sizeof(*ia_current_solution.array) * ia_current_solution.size);
                solutions->size += ia_current_solution.size;
                add_to_int_array(solutions, -1);
                sol_id_local++;
            }
            
            --ia_current_solution.size;
            index = ia_current_solution.array[ia_current_solution.size];//Dépile le dernier élément qui ne sert à rien
            get_head_wh_buf(whbufs, &wbuf, &hbuf);
            puzzle_light_off_with_bufs(p, pa_empty.array[index].column, pa_empty.array[index].line, wbuf, hbuf, &empty_count);
            release_wh_buf(whbufs);
        }
        index++;
    }
    while((ia_current_solution.size > 0) || (index < pa_empty.size));

    add_to_int_array(solutions, -2);
    *sol_id = sol_id_local;
    free_wh_bufs(whbufs);
}


int verify_solution(lu_puzzle *p, position_array left, position_array right, position_array top, position_array bottom, position_array center)
{
    unsigned char boolean = 1;
    unsigned int index = 0, lbulb_count = 0;
    position pos;

    unsigned int width = p->width, height = p->height;

    //COINS
    //Coins haut gauche
    if(p->data[0] == lusq_1)
    {
        boolean = boolean && ((p->data[1] == lusq_lbulb) || (p->data[width] == lusq_lbulb));
    }

    //Coins haut droit
    if(p->data[width-1] == lusq_1)
    {
        boolean = boolean && ((p->data[width-2] == lusq_lbulb) || (p->data[2*width-1] == lusq_lbulb));
    }

    //Coins bottom gauche
    if(p->data[width*(height-1)] == lusq_1)
    {
        boolean = boolean && ((p->data[width*(height-2)] == lusq_lbulb) || (p->data[width*(height-1) + 1] == lusq_lbulb));
    }

    //Coins bottom right
    if(p->data[width*height - 1] == lusq_1)
    {
        boolean = boolean && ((p->data[width*(height-1) - 1] == lusq_lbulb) || (p->data[width*height - 2 + 1] == lusq_lbulb));
    }

    //Bord gauche
    for(index = 0; index < left.size && boolean ; index++)
    {
        pos = left.array[index];

        lbulb_count = (
                (p->data[(pos.line-1)*width] == lusq_lbulb) +
                (p->data[(pos.line)*width + 1] == lusq_lbulb) +
                (p->data[(pos.line+1)*width] == lusq_lbulb));

        boolean = boolean && (lbulb_count == p->data[pos.line * width + pos.column]);
    }
    
    //Bord droit
    for(index = 0; index < right.size && boolean ; index++)
    {
        pos = right.array[index];

        lbulb_count = (
                (p->data[(pos.line-1)*width] == lusq_lbulb) +
                (p->data[(pos.line)*width - 1] == lusq_lbulb) +
                (p->data[(pos.line+1)*width] == lusq_lbulb));

        boolean = boolean && (lbulb_count == p->data[pos.line * width + pos.column]);
    }

    //Bord haut
    for(index = 0; index < top.size && boolean ; index++)
    {
        pos = top.array[index];

        lbulb_count = (
                (p->data[(pos.line)*width + pos.column - 1] == lusq_lbulb) +
                (p->data[(pos.line)*width + pos.column + 1] == lusq_lbulb) +
                (p->data[(pos.line+1)*width + pos.column] == lusq_lbulb));

        boolean = boolean && (lbulb_count == p->data[pos.line * width + pos.column]);
    }

    //Bord bas
    for(index = 0; index < bottom.size && boolean ; index++)
    {
        pos = bottom.array[index];

        lbulb_count = (
                (p->data[(pos.line)*width + pos.column - 1] == lusq_lbulb) +
                (p->data[(pos.line)*width + pos.column + 1] == lusq_lbulb) +
                (p->data[(pos.line-1)*width + pos.column] == lusq_lbulb));

        boolean = boolean && (lbulb_count == p->data[pos.line * width + pos.column]);
    }

    for(index = 0; index < center.size && boolean ; index++)
    {
        pos = center.array[index];

        lbulb_count = (
                (p->data[(pos.line)*width + pos.column - 1] == lusq_lbulb) +
                (p->data[(pos.line)*width + pos.column + 1] == lusq_lbulb) +
                (p->data[(pos.line-1)*width + pos.column] == lusq_lbulb) +
                (p->data[(pos.line+1)*width + pos.column] == lusq_lbulb));
        boolean = boolean && (lbulb_count == p->data[pos.line * width + pos.column]);
    }

    return boolean;
}

int try_solution(lu_puzzle * p, position_array pa_empty, int * solution)
{
    int i = 0;
    unsigned int width = p->width;
    position pos;

    if(solution[i] == -2) return 0;

    while((solution[i] != -1) && (possible_to_light(p, pa_empty.array[solution[i]]) == 1))
    {
        pos = pa_empty.array[solution[i]];
        p->data[pos.line * width + pos.column] = lusq_lbulb;
        i++;
    }

    if(solution[i] != -1)
    {
        for(i = i-1 ; i >= 0 ; i--)
        {
            pos = pa_empty.array[solution[i]];
            p->data[pos.line * width + pos.column] = lusq_empty;
        }
    }
    return i;
}

int remove_solution(lu_puzzle * p, position_array pa_empty, int * solution)
{
    int i = 0;
    unsigned int width = p->width;
    position pos;

    while(solution[i] != -1)
    {
        pos = pa_empty.array[solution[i]];
        p->data[pos.line * width + pos.column] = lusq_empty;
        i++;
    }
    return i++;
}

void remove_impossible(lu_puzzle * p)
{
    unsigned int i = 0, size = p->width * p->height;
    for(i = 0 ; i < size ; i++)
    {
        if(p->data[i] == lusq_impossible) p->data[i] = lusq_empty;
    }
}



void write_solutions(lu_puzzle * p, position_array * classes, int_array * classes_solutions, unsigned int nb_classes, unsigned int *sol_id, FILE *fd)
{
    int i = 0;
    int sol_id_local = 0;
    int * stack = NULL;
    unsigned int stack_size = 0;

    if(*sol_id == 0)
    {
        puzzle_store(p,fd);
        *sol_id = 1;
        return;
    }
    remove_impossible(p);

    stack = (int*) malloc(sizeof(int) * nb_classes);
    
    while((stack_size > 0) || (classes_solutions[stack_size].array[i] != -2))
    {
        while((classes_solutions[stack_size].array[i] != -2) && (try_solution(p, classes[stack_size], &classes_solutions[stack_size].array[i]) == 0))
        {
            i++;
        }
        if(classes_solutions[stack_size].array[i] != -2)
        {
            stack[stack_size] = i;
            stack_size++;
            i = 0;
        }
        else
        {
            --stack_size;
            i = stack[stack_size];
            i += remove_solution(p, classes[stack_size], &classes_solutions[stack_size].array[i]) + 1;

        }
        if(stack_size == nb_classes)
        {
            if(verify_solution(p,left_,right_,top_,bottom_,center_) == 1)
            {
                puzzle_store(p,fd);
                sol_id_local++;
            }

            --stack_size;
            i = stack[stack_size];
            i += remove_solution(p, classes[stack_size], &classes_solutions[stack_size].array[i]) + 1;
        }
    }
    *sol_id = sol_id_local;
}


void solve_classes(lu_puzzle *p, position_array * pa_classes,
        position_array * pa_impossible_classes, unsigned int pa_classes_size,
        unsigned int *sol_id, FILE *fd) 
{
    unsigned int index = 0;
    int_array * classes_solutions;
    int nb_sol = 1;

    classes_solutions = (int_array*)malloc(sizeof(int_array) * pa_classes_size);
    for(index = 0; index < pa_classes_size ; index++)
    {
        solve(p, pa_classes[index] , pa_impossible_classes[index], &classes_solutions[index], sol_id);
        nb_sol *= *sol_id;
    }
    
    char * buf = (char*)malloc(sizeof(char) * p->width * p->height * nb_sol);
    setvbuf(fd, buf, _IOFBF, sizeof(char) * p->width * p->height * nb_sol);

    write_solutions(p, pa_classes, classes_solutions, pa_classes_size, sol_id, fd);
}
