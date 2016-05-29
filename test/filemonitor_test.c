#include "../peer/filemonitor.h"

/*
void myFileAdded(char* filename){
	printf("Here, %s", filename);
}
void(*fileAdded)(char*) = myFileAdded;
*/

// TEST FUNCTION
int main( ){
  char* path = readConfigFile("./peer/config.ini");
  FileTable *filetable = initTable(path);
  watchDirectory(path);
  printf("Files under the directory:\n");
  printAllFilesInfo();

  pthread_t t;
  pthread_create(&t, NULL, monitor, (void*) filetable);
  while(1){
    sleep(1);
  }
}
