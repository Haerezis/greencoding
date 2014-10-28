#include "heuristics.h"

char wall_heuristic(lu_puzzle *p,
        position_array * pa_left_border,
        position_array * pa_right_border,
        position_array * pa_top_border,
        position_array * pa_bottom_border,
        position_array * pa_center)
{
    unsigned int c = 0,
                 l = 0,
                 count = 0,
                 current_state = 0,
                 index = 0;

    unsigned char change = 0;
    
    unsigned int width = p->width;
    unsigned int height = p->height;

    position_array pa_local_center = *pa_center,
                   pa_local_left_border = *pa_left_border,
                   pa_local_right_border = *pa_right_border,
                   pa_local_top_border = *pa_top_border,
                   pa_local_bottom_border = *pa_bottom_border;

    //COINS
    //Coin haut gauche
    current_state = p->data[0];
    if(current_state == lusq_0)
    {
        if(p->data[1] == lusq_empty)
        {
            p->data[1] = lusq_impossible;
            change = 1;
        }
        if(p->data[width] == lusq_empty)
        {
            p->data[width] = lusq_impossible;
            change = 1;
        }
    }
    else if(current_state <= lusq_2)
    {
        count =
            (p->data[1] == lusq_empty) +
            (p->data[width] == lusq_empty) +
            (p->data[1] == lusq_lbulb) +
            (p->data[width] == lusq_lbulb);
        if(count == current_state)
        {
            if(p->data[1] == lusq_empty)
            {
                puzzle_light_on(p,1,0);
                change = 1;
            }
            if(p->data[width] == lusq_empty)
            {
                puzzle_light_on(p,0,1);
                change = 1;
            }
        }
    }
    //Coin haut droit
    current_state = p->data[width-1];
    if(current_state == lusq_0)
    {
        if(p->data[width-2] == lusq_empty)
        {
            p->data[width-2] = lusq_impossible;
            change = 1;
        }
        if(p->data[2*width - 1] == lusq_empty)
        {
            p->data[2*width - 1] = lusq_impossible;
            change = 1;
        }
    }
    else if(current_state <= lusq_2)
    {
        count =
            (p->data[width-2] == lusq_empty) +
            (p->data[2*width - 1] == lusq_empty) +
            (p->data[width-2] == lusq_lbulb) +
            (p->data[2*width - 1] == lusq_lbulb);
        if(count == current_state)
        {
            if(p->data[width-2] == lusq_empty) 
            {
                puzzle_light_on(p,width-2,0);
                change = 1;
            }
            if(p->data[2*width - 1] == lusq_empty)
            {
                puzzle_light_on(p,width-1,1);
                change = 1;
            }
        }
    }
    //Coin bas gauche
    current_state = p->data[width*(height-1)];
    if(current_state == lusq_0)
    {
        if(p->data[width*(height-2)] == lusq_empty)
        {
            p->data[width*(height-2)] = lusq_impossible;
            change = 1;
        }
        if(p->data[width*(height-1) + 1] == lusq_empty)
        {
            p->data[width*(height-1) + 1] = lusq_impossible;
            change = 1;
        }
    }
    else if(current_state <= lusq_2)
    {
        count =
            (p->data[width*(height-2)] == lusq_empty) +
            (p->data[width*(height-1) + 1] == lusq_empty) +
            (p->data[width*(height-2)] == lusq_lbulb) +
            (p->data[width*(height-1) + 1] == lusq_lbulb);
        if(count == current_state)
        {
            if(p->data[width*(height-2)] == lusq_empty)
            {
                puzzle_light_on(p,0,height-2);
                change = 1;
            }
            if(p->data[width*(height-1) + 1] == lusq_empty)
            {
                puzzle_light_on(p,1,height-1);
                change = 1;
            }
        }
    }
    //Coin bas droit
    current_state = p->data[width*height - 1];
    if(current_state == lusq_0)
    {
        if(p->data[width*height - 2] == lusq_empty)
        {
            p->data[width*height - 2] = lusq_impossible;
            change = 1;
        }
        if(p->data[width*(height-1) - 1] == lusq_empty)
        {
            p->data[width*(height-1) - 1] = lusq_impossible;
            change = 1;
        }
    }
    else if(current_state <= lusq_2)
    {
        count =
            (p->data[width*height - 2] == lusq_empty) +
            (p->data[width*(height-1) - 1] == lusq_empty) +
            (p->data[width*height - 2] == lusq_lbulb) +
            (p->data[width*(height-1) - 1] == lusq_lbulb);
        if(count == current_state)
        {
            if(p->data[width*height - 2] == lusq_empty)
            {
                puzzle_light_on(p,width-2,height-1);
                change = 1;
            }
            if(p->data[width*(height-1) - 1] == lusq_empty)
            {
                puzzle_light_on(p,width-1,height-2);
                change = 1;
            }
        }
    }

    //BORD SANS LES COINS/////////
    //BORD GAUCHE ET DROIT
    for (index = 0; index < pa_local_left_border.size ; ++index)
    {
        l = pa_local_left_border.array[index].line;
        current_state = p->data[l * width];
        if(current_state == lusq_0 || wall_saturated(p,0,l))
        {
            if(p->data[(l-1) * width] == lusq_empty) p->data[(l-1) * width] = lusq_impossible;
            if(p->data[(l+1) * width] == lusq_empty) p->data[(l+1) * width] = lusq_impossible;
            if(p->data[l * width + 1] == lusq_empty) p->data[l * width + 1] = lusq_impossible;
            remove_from_position_array(&pa_local_left_border,index);
            change = 1;
        }
        else if(current_state <= lusq_3)
        {
            count =
                (p->data[(l-1) * width] == lusq_empty) +
                (p->data[(l+1) * width] == lusq_empty) +
                (p->data[l * width + 1] == lusq_empty) +
                (p->data[(l-1) * width] == lusq_lbulb) +
                (p->data[(l+1) * width] == lusq_lbulb) +
                (p->data[l * width + 1] == lusq_lbulb);
            if(count == current_state)
            {
                if(p->data[(l-1) * width] == lusq_empty) puzzle_light_on(p,0,l-1);
                if(p->data[(l+1) * width] == lusq_empty) puzzle_light_on(p,0,l+1);
                if(p->data[l * width + 1] == lusq_empty) puzzle_light_on(p,1,l);
                remove_from_position_array(&pa_local_left_border,index);
                index--;
                change = 1;
            }
        }
    }
    for (index = 0; index < pa_local_right_border.size ; ++index)
    {
        l = pa_local_right_border.array[index].line;
        current_state = p->data[l * width + width -1];
        if(current_state == lusq_0 || wall_saturated(p,width-1,l))
        {
            if(p->data[(l-1) * width + width-1] == lusq_empty) p->data[(l-1) * width + width-1] = lusq_impossible;
            if(p->data[(l+1) * width + width-1] == lusq_empty) p->data[(l+1) * width + width-1] = lusq_impossible;
            if(p->data[l * width + width-2] == lusq_empty) p->data[l * width + width-2] = lusq_impossible;
            remove_from_position_array(&pa_local_right_border,index);
            change = 1;
        }
        else if(current_state <= lusq_3)
        {
            count =
                (p->data[(l-1) * width + width-1] == lusq_empty) +
                (p->data[(l+1) * width + width-1] == lusq_empty) +
                (p->data[l * width + width-2] == lusq_empty) +
                (p->data[(l-1) * width + width-1] == lusq_lbulb) +
                (p->data[(l+1) * width + width-1] == lusq_lbulb) +
                (p->data[l * width + width-2] == lusq_lbulb);
            if(count == current_state)
            {
                if(p->data[(l-1) * width + width-1] == lusq_empty) puzzle_light_on(p,width-1,l-1);
                if(p->data[(l+1) * width + width-1] == lusq_empty) puzzle_light_on(p,width-1,l+1);
                if(p->data[l * width + width-2] == lusq_empty) puzzle_light_on(p,width-2,l);
                remove_from_position_array(&pa_local_right_border,index);
                index--;
                change = 1;
            }
        }
    }
    //BORD HAUT ET BAS
    for (index = 0; index < pa_local_top_border.size ; ++index)
    {
        c = pa_local_top_border.array[index].column;
        current_state = p->data[c];
        if(current_state == lusq_0 || wall_saturated(p,c,0))
        {
            if(p->data[c-1] == lusq_empty) p->data[c-1] = lusq_impossible;
            if(p->data[c+1] == lusq_empty) p->data[c+1] = lusq_impossible;
            if(p->data[width + c] == lusq_empty) p->data[width + c] = lusq_impossible;
            remove_from_position_array(&pa_local_top_border,index);
            change = 1;
        }
        else if(current_state <= lusq_3)
        {
            count =
                (p->data[c-1] == lusq_empty) +
                (p->data[c+1] == lusq_empty) +
                (p->data[width + c] == lusq_empty) +
                (p->data[c-1] == lusq_lbulb) +
                (p->data[c+1] == lusq_lbulb) +
                (p->data[width + c] == lusq_lbulb);
            if(count == current_state)
            {
                if(p->data[c-1] == lusq_empty) puzzle_light_on(p,c-1,0);
                if(p->data[c+1] == lusq_empty) puzzle_light_on(p,c+1,0);
                if(p->data[width + c] == lusq_empty) puzzle_light_on(p,c,1);
                remove_from_position_array(&pa_local_top_border,index);
                index--;
                change = 1;
            }
        }
    }
    for (index = 0; index < pa_local_bottom_border.size ; ++index)
    {
        c = pa_local_bottom_border.array[index].column;
        current_state = p->data[(height-1)*width + c];
        if(current_state == lusq_0 || wall_saturated(p,c,height-1))
        {
            if(p->data[(height-1)*width + c-1] == lusq_empty) p->data[(height-1)*width + c-1] = lusq_impossible;
            if(p->data[(height-1)*width + c+1] == lusq_empty) p->data[(height-1)*width + c+1] = lusq_impossible;
            if(p->data[(height-2)*width + c] == lusq_empty) p->data[(height-2)*width +  + c] = lusq_impossible;
            remove_from_position_array(&pa_local_bottom_border,index);
            change = 1;
        }
        else if(current_state <= lusq_3)
        {
            count =
                (p->data[(height-1)*width + c-1] == lusq_empty) +
                (p->data[(height-1)*width + c+1] == lusq_empty) +
                (p->data[(height-2)*width + c] == lusq_empty) +
                (p->data[(height-1)*width + c-1] == lusq_lbulb) +
                (p->data[(height-1)*width + c+1] == lusq_lbulb) +
                (p->data[(height-2)*width + c] == lusq_lbulb);
            if(count == current_state)
            {
                if(p->data[(height-1)*width + c-1] == lusq_empty) puzzle_light_on(p,c-1,height-1);
                if(p->data[(height-1)*width + c+1] == lusq_empty) puzzle_light_on(p,c+1,height-1);
                if(p->data[(height-2)*width + c] == lusq_empty) puzzle_light_on(p,c,height-2);
                remove_from_position_array(&pa_local_bottom_border,index);
                index--;
                change = 1;
            }
        }
    }

    //INTERIEUR SANS BORD NI COINS
    for (index = 0; index < pa_local_center.size ; ++index)
    {
        l = pa_local_center.array[index].line;
        c = pa_local_center.array[index].column;
        current_state = p->data[l * width + c];
        if(current_state == lusq_0 || wall_saturated(p,c,l))
        {
            if(p->data[(l-1) * width + c] == lusq_empty) p->data[(l-1) * width + c] = lusq_impossible;
            if(p->data[(l+1) * width + c] == lusq_empty) p->data[(l+1) * width + c] = lusq_impossible;
            if(p->data[l * width + c - 1] == lusq_empty) p->data[l * width + c - 1] = lusq_impossible;
            if(p->data[l * width + c + 1] == lusq_empty) p->data[l * width + c + 1] = lusq_impossible;
            remove_from_position_array(&pa_local_center,index);
            change = 1;
        }
        else if(current_state <= lusq_4)
        {
            count = 
                (p->data[(l-1) * width + c] == lusq_empty) +
                (p->data[(l+1) * width + c] == lusq_empty) +
                (p->data[l * width + c - 1] == lusq_empty) +
                (p->data[l * width + c + 1] == lusq_empty) +
                (p->data[(l-1) * width + c] == lusq_lbulb) +
                (p->data[(l+1) * width + c] == lusq_lbulb) +
                (p->data[l * width + c - 1] == lusq_lbulb) +
                (p->data[l * width + c + 1] == lusq_lbulb);
            if(count == current_state)
            {
                if(p->data[(l-1) * width + c] == lusq_empty) puzzle_light_on(p,c,l-1);
                if(p->data[(l+1) * width + c] == lusq_empty) puzzle_light_on(p,c,l+1);
                if(p->data[l * width + c - 1] == lusq_empty) puzzle_light_on(p,c-1,l);
                if(p->data[l * width + c + 1] == lusq_empty) puzzle_light_on(p,c+1,l);
                remove_from_position_array(&pa_local_center,index);
                index--;
                change = 1;
            }
        }
    }

    
    *pa_center = pa_local_center;
    *pa_left_border = pa_local_left_border;
    *pa_right_border = pa_local_right_border;
    *pa_top_border = pa_local_top_border; 
    *pa_bottom_border = pa_local_bottom_border;

    return change;
}


char empty_and_impossible_heuristic(lu_puzzle * p, position_array * pa_empty, position_array * pa_impossible)
{
    unsigned int c = 0,
                 l = 0,
                 index = 0;

    unsigned char change = 0;
    
    unsigned int width = p->width;

    position_array pa_local_empty = *pa_empty,
                   pa_local_impossible = *pa_impossible;


    position lb_pos = {0,0};
    for(index = 0; index < pa_local_impossible.size ; index++)
    {
        l = pa_local_impossible.array[index].line;
        c = pa_local_impossible.array[index].column;
    
        if(p->data[l * width + c] != lusq_impossible)
        {
            remove_from_position_array(&pa_local_impossible, index);
            index--;
        }
        else if(number_of_lightbulb_possible(p, l, c, &lb_pos) == 1)
        {
            puzzle_light_on(p, lb_pos.column, lb_pos.line);
            remove_from_position_array(&pa_local_impossible, index);
            index--;
            change = 1;
        }
    }
    //Essaye de remplir les cases vides qui doivent s'éclairer eux même (lorsqu'aucune case n'est libre autour d'eux pour l'éclairer).
    for(index = 0; index < pa_local_empty.size ; index++)
    {
        l = pa_local_empty.array[index].line;
        c = pa_local_empty.array[index].column;
    
        if(p->data[l * width + c] != lusq_empty)
        {
            if(p->data[l * width + c] == lusq_impossible)
            {
                add_to_position_array(&pa_local_impossible, pa_local_empty.array[index]);
                change = 1;
            }
            remove_from_position_array(&pa_local_empty, index);
            index--;
        }
        else if(is_alone(p, l, c) == 1)
        {
            puzzle_light_on(p, pa_local_empty.array[index].column, pa_local_empty.array[index].line);
            remove_from_position_array(&pa_local_empty, index);
            index--;
            change = 1;
        }
    }

    *pa_empty = pa_local_empty;
    *pa_impossible = pa_local_impossible;

    return change;
}



void pre_solve(lu_puzzle *p, position_array * positions_empty,
        position_array * positions_impossible,position_array * left,
        position_array * right, position_array * top, position_array * bottom,
        position_array * center)
{
    unsigned int c = 0,
                 l = 0,
                 index = 0;

    unsigned int change = 0;
    
    unsigned int width = p->width;
    unsigned int height = p->height;

    
    position_array pa_empty = new_position_array();
    position_array pa_impossible = new_position_array();

    position_array pa_center = new_position_array_with_size((width-2)*(height-2));
    position_array pa_left_border = new_position_array_with_size(height);
    position_array pa_right_border = new_position_array_with_size(height);
    position_array pa_top_border = new_position_array_with_size(width);
    position_array pa_bottom_border = new_position_array_with_size(width);



    //Filling the array of wall with number on it
    index = 0;
    for (l = 1; l < p->height-1; ++l)
    {
        for (c = 1; c < p->width -1; ++c)
        {
            pa_center.array[index] = (position){l,c};
            index += (p->data[l * width + c] <= lusq_4);
        }
    }
    pa_center.size = index;

    index = 0;
    for (l = 1; l < height-1; ++l)
    {
        pa_left_border.array[index] = (position){l,0};
        index += (p->data[l * width] <= lusq_4);
    }
    pa_left_border.size = index;
    index = 0;
    for (l = 1; l < height-1; ++l)
    {
        pa_right_border.array[index] = (position){l,width-1};
        index += (p->data[l * width + width -1] <= lusq_4);
    }
    pa_right_border.size = index;
    index = 0;
    for (c = 1; c < width-1; ++c)
    {
        pa_top_border.array[index] = (position){0,c};
        index += (p->data[c] <= lusq_4);
    }
    pa_top_border.size = index;
    index = 0;
    for (c = 1; c < width-1; ++c)
    {
        pa_bottom_border.array[index] = (position){height-1,c};
        index += (p->data[(height-1)*width + c] <= lusq_4);
    }
    pa_bottom_border.size = index;

    /////////////////////////////////////////
    //APPEL A LA FONCTION HEURISTIC QUI REMPLIE LES CASES VIDES SELON LES CONTRAINTES DES MURS
    /////////////////////////////////////////
    do
    {
        change = wall_heuristic(
                p, 
                &pa_left_border,
                &pa_right_border,
                &pa_top_border,
                &pa_bottom_border,
                &pa_center);
    }
    while(change != 0);


    unsigned index_empty = 0, index_impossible = 0;
    for (l = 0; l < p->height; ++l)
    {
        for (c = 0; c < p->width; ++c)
        {
            pa_empty.array[index_empty] = (position){l,c};
            pa_impossible.array[index_impossible] = (position){l,c};
            index_empty += (p->data[l * width + c] == lusq_empty);
            index_impossible +=(p->data[l * width + c] == lusq_impossible);
        }
    }
    pa_empty.size = index_empty;
    pa_impossible.size = index_impossible;

    
    do
    {
        change = 0;

        change += empty_and_impossible_heuristic(
                p, 
                &pa_empty,
                &pa_impossible);

        /////////////////////////////////////////
        //APPEL A LA FONCTION HEURISTIC QUI REMPLIE LES CASES VIDES SELON LES CONTRAINTES DES MURS
        /////////////////////////////////////////
        change += wall_heuristic(
                p, 
                &pa_left_border,
                &pa_right_border,
                &pa_top_border,
                &pa_bottom_border,
                &pa_center);
    }
    while(change != 0);




    *positions_empty = pa_empty;
    *positions_impossible = pa_impossible;
    *left = pa_left_border;
    *right = pa_right_border;
    *top = pa_top_border;
    *bottom = pa_bottom_border;
    *center = pa_center;
}
