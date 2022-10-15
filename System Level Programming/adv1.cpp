#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <map>

volatile size_t counter = 0;
pthread_spinlock_t array_lock;
ssize_t array[16];
ssize_t empty_elements_in_array = 16;
ssize_t full_elements_in_array = 0;
size_t run = 1;

void producer(size_t x)
{
  for (size_t i = 0; i < 100000ULL; ++i)
  {
    while (1)
    {
      for (size_t j = 0; j < 16; ++j)
      {
        pthread_spin_lock(&array_lock);
        if (array[j] == -1)
        {
          array[j] = x;
          usleep(1);
          empty_elements_in_array--;
          full_elements_in_array++;
          pthread_spin_unlock(&array_lock);
          goto next_one;
        }
        pthread_spin_unlock(&array_lock);
      }
    }
    next_one: ;
  }
}

void consumer(size_t x)
{
  pthread_spin_lock(&array_lock);
  while (run || full_elements_in_array > 0)
  {
    pthread_spin_unlock(&array_lock);
    //size_t consumed_something = 0;
    for (size_t i = 0; i < 16; ++i)
    {
      pthread_spin_lock(&array_lock);
      if (array[i] != -1)
      {
        counter += array[i];
        //consumed_something = 1;
        array[i] = -1;
        empty_elements_in_array++;
        full_elements_in_array--;
        pthread_spin_unlock(&array_lock);
        break;
      }
      pthread_spin_unlock(&array_lock);
    }
/*    if (consumed_something == 0)
    {
      printf(".");
      fflush(stdout);
    }*/
    pthread_spin_lock(&array_lock);
  }
  pthread_spin_unlock(&array_lock);
}

int main()
{
  pthread_spin_init(&array_lock,0);
  memset(array,-1,16*sizeof(ssize_t));
  pthread_t producers[10];
  for (size_t i = 0; i < 3; ++i)
    pthread_create(producers+i,0,(void*(*)(void*))&producer,(void *)1);
  pthread_t consumers[10];
  for (size_t i = 0; i < 3; ++i)
    pthread_create(consumers+i,0,(void*(*)(void*))&consumer,(void *)1);
  for (size_t i = 0; i < 3; ++i)
    pthread_join(producers[i],0);
  printf("all producers are done\n");
  run = 0;
  for (size_t i = 0; i < 3; ++i)
    pthread_join(consumers[i],0);
  printf("counter = %zu\n", counter);
  return 0;
}
