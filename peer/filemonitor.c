#include "filemonitor.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

char* dirPath;
int fd;
int wd;

/*
* Begin to watch a directory using inotify
* and assign descroptors to fd,wd.
*/
void watchDirectory(char* directory){
  fd = inotify_init();
  if (fd < 0){
    perror("inotify_init");
  }
  wd = inotify_add_watch(fd, directory, IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_TO | IN_MOVED_FROM | IN_ATTRIB);

  // ALSO START MONITOR-THREAD
}

/*
* Read a config file from a disk
* Config file stores a directory path to be monitored
* 
*/
char* readConfigFile(char* filename){
  FILE *fp;
  fp = fopen(filename, "r");
  if (fp == NULL){
    printf("Failed to open config file.\n");
    return NULL;
  }

  dirPath = (char*) malloc(sizeof(char)*128);
  memset(dirPath, 0, 128);
  char buf[128] = {0};
  memset(buf, 0, 128);
  fgets(buf, 128, fp);
  char* temp;
  temp = strtok(buf, " ");
  strcpy(dirPath, temp);
  //dirPath[5] = '\0';
  //dirPath = "./tmp";
  return dirPath;
}

/*
* Get all files' info in a directory
* Returns a pointer to FileInfoList containing a list of FileInfo
* and the total number of files
*/
FileInfoList* getAllFilesInfo(){
  FileInfoList* files;
  char fullpath[1024];
  files = (FileInfoList*) malloc(sizeof(FileInfoList));
  files->list = (FileInfo**) malloc(sizeof(FileInfo*)*MAX_FILES);
  files->length = 0;
  DIR* d;
  struct dirent *dir;
  struct stat st;

  d = opendir(dirPath);
  if (d == NULL){
    printf("%s\n", strerror(errno));
  }
  
  if (d){
    while ((dir = readdir(d)) != NULL){
      sprintf(fullpath, "%s/%s", dirPath, dir->d_name);
      stat(fullpath, &st);
      if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")){
        continue;
      }
      // CURRENTLY ONLY SUPPORT REGULAR FILES
      if (S_ISREG(st.st_mode)){
        files->list[files->length] = (FileInfo*) malloc(sizeof(FileInfo));
        files->list[files->length]->filepath = (char*) malloc(sizeof(char)*LEN_FILE_NAME);
        memcpy(files->list[files->length]->filepath, dir->d_name, sizeof(char)*LEN_FILE_NAME);
        files->list[files->length]->size = (long) st.st_size;
        files->list[files->length]->lastModifyTime = (long) st.st_ctime;
        files->length++;
      }
    }
  }
  closedir(d);
  return files;
}

/*
* Free up memory allocated for a given FileInfoList
*/
void freeFileInfoList(FileInfoList* files){
  int i;
  for (i=0; i<files->length; i++){
    free(files->list[i]->filepath);
  }
  free(files);
}

/*
* Print all files' info in terminal for debug purpose
*/
void printAllFilesInfo(){
  FileInfoList* files = getAllFilesInfo();
  int i;
  for (i=0; i<files->length; i++){
    printf("Name:%s, Size:%ld, Timestamp:%ld \n", files->list[i]->filepath, files->list[i]->size, files->list[i]->lastModifyTime);
  }
  freeFileInfoList(files);
}

/*
* Get a FileInfo by giving the filename of a file
*/
FileInfo* getFileInfo(char* filename){
  FileInfo* file;
  char fullpath[LEN_FILE_NAME];
  file = (FileInfo*) malloc(sizeof(FileInfo));
  file->filepath = (char*) malloc(sizeof(char)*LEN_FILE_NAME);
  sprintf(fullpath, "%s/%s", dirPath, filename);
  struct stat st;
  stat(fullpath, &st);
  memcpy(file->filepath, filename, sizeof(char)*LEN_FILE_NAME);
  file->size = (long) st.st_size;
  file->lastModifyTime = (long) st.st_ctime;
  return file;
}

/*
* Check to see if a file previously existed in the directory
*/
int isInFileInfoList(char* filename, FileInfoList* files){
  int i;
  for (i=0; i<files->length; i++){
    if (!strcmp(files->list[i]->filepath, filename)){
      return 1;
      break;
    }
  }
  return 0;
}

void* monitor(void* arg){
  int length, i = 0, isInDir;
  char buffer[EVENT_BUF_LEN];
  while (1){
    FileInfoList* files = getAllFilesInfo();
    length = read( fd, buffer, EVENT_BUF_LEN ); 

    if ( length < 0 ) {
      perror( "read" );
    }  

    i=0;
    while ( i < length ) {     
      struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
      if ( event->len ) {
        isInDir = isInFileInfoList(event->name, files);

        if (*(event->name)=='.'){
          // IGNORE AS IT IS HIDDEN FILE
        }else if ( event->mask & IN_CREATE ) {
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            //fileAdded(event->name);
            printf( " File %s added.\n", event->name );
          }
        }
        else if ( event->mask & IN_DELETE ) {
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            //fileDeleted(event->name);
            printf( " File %s deleted.\n", event->name );
          }
        }
        else if (event->mask & IN_MODIFY){
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            //fileModified(event->name);
            printf( " File %s modified.\n", event->name );
          }
        }
        else if (event->mask & IN_MOVED_FROM){
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            //fileDeleted(event->name);
            printf( " File %s deleted.\n", event->name );
          }
        }
        else if (event->mask & IN_MOVED_TO){
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            if (isInDir){
              //fileModified(event->name);
              printf( " File %s modified.\n", event->name );
            }else{
              //fileAdded(event->name);
              printf( " File %s added.\n", event->name );
            }
            
          }
        }
        else if (event->mask & IN_ATTRIB){
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            //fileModified(event->name);
            if (isInDir){
              printf( " File %s modified.\n", event->name );

            }
          }
        }


      }
      i += EVENT_SIZE + event->len;
    }
  }
  inotify_rm_watch( fd, wd );
  close( fd );

}

