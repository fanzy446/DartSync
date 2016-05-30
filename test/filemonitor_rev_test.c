#include "../peer/filemonitor_rev.h"


// TEST FUNCTION
int main( ){
  char* path = readConfigFile("./config.ini");
  FileTable *filetable = createTable();// initTable(path);
  watchDirectory(path);
  listDir(filetable, path);
  printTable(filetable);

  pthread_mutex_t *filetable_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(filetable_mutex, NULL);
  filemonitorArg_st *args = malloc(sizeof(filemonitorArg_st));
  args->filetable = filetable;
  args->filetable_mutex = filetable_mutex;
  args->trackerconn = 0;

  pthread_t t;
  pthread_create(&t, NULL, monitor, (void*) args);
  int i=0;
  while(1){
    if (i++ == 60){
      break;
    }
    sleep(1);
  }
  destroyTable(filetable);

}
