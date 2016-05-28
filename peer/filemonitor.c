#include "filemonitor.h"
#include "../common/filetable.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

char* dirPath;
int fd;
int wd;

pthread_mutex_t add_lock;
pthread_mutex_t write_lock;
pthread_mutex_t delete_lock;
int add_listening_enabled = 1;
int write_listening_enabled = 1;
int delete_listening_enabled = 1;

/*
* INTERFACES
*/

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

  if(pthread_mutex_init(&add_lock, NULL) != 0 || pthread_mutex_init(&write_lock, NULL) != 0 || pthread_mutex_init(&delete_lock, NULL) != 0){
    printf(" mutex init failed\n");
  }

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
 // memcpy(file->filepath, filename, sizeof(char)*LEN_FILE_NAME);
  strcpy(file->filepath, filename);
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


/*
* Thread to monitor changes in files under a directory
*/
void* monitor(void* arg){
  //extract information from arg
  struct filemonitorArgs *args = (struct filemonitorArgs *) arg;
  FileTable *table = args->filetable;
  pthread_mutex_t *filetable_mutex = args->filetable_mutex;
  int trackerconn = args->trackerconn;

  // takes filetable as an arg
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
        freeFileInfoList(files);

        if (*(event->name)=='.'){
          // IGNORE AS IT IS HIDDEN FILE
        }else if ( event->mask & IN_CREATE ) {
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            if (add_listening_enabled){
              fileAdded(table, event->name, filetable_mutex, trackerconn);
              printf( " File %s added.\n", event->name );
            }
          }
        }
        else if ( event->mask & IN_DELETE ) {
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            if (delete_listening_enabled){
              fileDeleted(table, event->name, filetable_mutex, trackerconn);
              printf( " File %s deleted.\n", event->name );
            }
          }
        }
        else if (event->mask & IN_MODIFY){
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            if (write_listening_enabled){
              fileModified(table, event->name, filetable_mutex, trackerconn);
              printf( " File %s modified.\n", event->name );
            }
          }
        }
        else if (event->mask & IN_MOVED_FROM){
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            if (delete_listening_enabled){
              fileDeleted(table, event->name, filetable_mutex, trackerconn);
              printf( " File %s deleted.\n", event->name );
            }
          }
        }
        else if (event->mask & IN_MOVED_TO){
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            if (isInDir){
              if (write_listening_enabled){
                fileModified(table, event->name, filetable_mutex, trackerconn);
                printf( " File %s modified.\n", event->name );
              }
            }else{
              if (add_listening_enabled){
                fileAdded(table, event->name, filetable_mutex, trackerconn);
                printf( " File %s added.\n", event->name );
              }
            }
          }
        }
        else if (event->mask & IN_ATTRIB){
          if ( event->mask & IN_ISDIR ) {
            // IGNORE FOR NOW AS WE DON'T KEEP TRACK DIRECTORIES
          }
          else {
            if (isInDir){
              if (write_listening_enabled){
                fileModified(table, event->name, filetable_mutex, trackerconn);
                printf( " File %s modified.\n", event->name );
              }
            }
          }
        }

      }
      i += EVENT_SIZE + event->len;
    }
    //printTable(table); // for debug purpose
  }
  inotify_rm_watch( fd, wd );
  close( fd );

}

/*
* SUPPORT FUNCTIONS
*/

void fileAdded(FileTable* table, char* filename, pthread_mutex_t *filetable_mutex, int trackerconn){
  FileInfo *fi = getFileInfo(filename);
  pthread_mutex_lock(filetable_mutex);
  addNewNode(table, fi->filepath, fi->size, fi->lastModifyTime, NULL);
  pthread_mutex_unlock(filetable_mutex);
  sendFileUpdate(table, filetable_mutex, trackerconn);
}

void fileModified(FileTable* table, char* filename, pthread_mutex_t *filetable_mutex, int trackerconn){
  FileInfo *fi = getFileInfo(filename);
  pthread_mutex_lock(filetable_mutex);
  modifyNode(table, fi->filepath, fi->size, fi->lastModifyTime, NULL);
  pthread_mutex_unlock(filetable_mutex);
  sendFileUpdate(table, filetable_mutex, trackerconn);
}

void fileDeleted(FileTable* table, char* filename, pthread_mutex_t *filetable_mutex, int trackerconn){
  pthread_mutex_lock(filetable_mutex);
  deleteNode(table, filename);
  pthread_mutex_unlock(filetable_mutex);
  sendFileUpdate(table, filetable_mutex, trackerconn);
}

void blockFileAddListening(){
  pthread_mutex_lock(&add_lock);
  add_listening_enabled = 0;
  pthread_mutex_unlock(&add_lock);
}
void unblockFileAddListening(){
  pthread_mutex_lock(&add_lock);
  add_listening_enabled = 1;
  pthread_mutex_unlock(&add_lock);
}
void blockFileWriteListening(){
  pthread_mutex_lock(&write_lock);
  write_listening_enabled = 0;
  pthread_mutex_unlock(&write_lock);
}
void unblockFileWriteListening(){
  pthread_mutex_lock(&write_lock);
  write_listening_enabled = 1;
  pthread_mutex_unlock(&write_lock);
}
void blockFileDeleteListening(){
  pthread_mutex_lock(&delete_lock);
  delete_listening_enabled = 0;
  pthread_mutex_unlock(&delete_lock);
}
void unblockFileDeleteListening(){
  pthread_mutex_lock(&delete_lock);
  delete_listening_enabled = 1;
  pthread_mutex_unlock(&delete_lock);
}

int sendFileUpdate(FileTable *filetable, pthread_mutex_t *filetable_mutex, int trackerconn){
  ptp_peer_t fileUpdate;
  fileUpdate.type = FILE_UPDATE; 
  memcpy(fileUpdate.peer_ip, getMyIP(), IP_LEN);
  packFileTable(filetable, filetable_mutex, fileUpdate.sendNode, &fileUpdate.file_table_size);
  if (peer_sendseg(trackerconn, &fileUpdate) < 0){
    printf("Failed to send file update packet\n");
    return -1; 
  }
  else{
    printf("File update packet sent\n");
    return 1; 
  }
}


