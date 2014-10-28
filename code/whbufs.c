#include "whbufs.h"

wh_bufs *new_wh_bufs(unsigned int width, unsigned int height, unsigned int size) {
   wh_bufs *res = (wh_bufs *) malloc(sizeof(*res));

   res->width = width;
   res->height = height;
   res->nb_used = 0;
   res->nb_allocs = (size > 0) ? size : width + height;    // estimation du nb d'ampoules

   res->wbuf = (char *) malloc(sizeof(*res->wbuf) * res->nb_allocs * width);
   res->hbuf = (char *) malloc(sizeof(*res->hbuf) * res->nb_allocs * height);

   return res;
}


void pop_wh_buf(wh_bufs *bufs, char **wbuf, char **hbuf) {
   // tous les buffers sont utilisés
   if (bufs->nb_used >= bufs->nb_allocs) {
      bufs->nb_allocs *= 2;

      bufs->wbuf = (char *) realloc(
              bufs->wbuf,
              sizeof(*bufs->wbuf) * bufs->width * bufs->nb_allocs);
      bufs->hbuf = (char *) realloc(
              bufs->hbuf,
              sizeof(*bufs->hbuf) * bufs->height * bufs->nb_allocs);
   }

   *wbuf = bufs->wbuf;
   bufs->wbuf += bufs->width;
   *hbuf = bufs->hbuf;
   bufs->hbuf += bufs->height;
   ++bufs->nb_used;
   
   memset(*wbuf, 0, sizeof(**wbuf) * bufs->width);
   memset(*hbuf, 0, sizeof(**hbuf) * bufs->height);
}

void get_head_wh_buf(wh_bufs *bufs, char **wbuf, char **hbuf)
{
   if(bufs->nb_used == 0)
   {
       *wbuf = NULL;
       *hbuf = NULL;
   }
   else
   {
       *wbuf = bufs->wbuf - bufs->width;
       *hbuf = bufs->hbuf - bufs->height;
   }
}

void release_wh_buf(wh_bufs *bufs) {
   if (bufs->nb_used == 0) {
      return;
   }

    bufs->wbuf = bufs->wbuf - bufs->width;
    bufs->hbuf = bufs->hbuf - bufs->height;

   --bufs->nb_used;
}

void free_wh_bufs(wh_bufs *bufs) {
   free(bufs->wbuf - bufs->nb_used*bufs->width);
   free(bufs->hbuf - bufs->nb_used*bufs->height);
   free(bufs);
}
