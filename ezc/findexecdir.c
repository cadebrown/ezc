
/*

Taken from: https://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe

Licensed under MIT

*/

#include "findexecdir.h"
#include <libgen.h>
#include <string.h>


char findexecdir_save_pwd[PATH_MAX];
char findexecdir_save_argv0[PATH_MAX];
char findexecdir_save_path[PATH_MAX];
char findexecdir_path_separator='/';
char findexecdir_path_separator_as_string[2]="/";
char findexecdir_path_list_separator[8]=":";  // could be ":; "
char findexecdir_debug=0;

int findexecdir_initialized=0;

void findexecdir_init(char *argv0) {
  getcwd(findexecdir_save_pwd, sizeof(findexecdir_save_pwd));

  strncpy(findexecdir_save_argv0, argv0, sizeof(findexecdir_save_argv0));
  findexecdir_save_argv0[sizeof(findexecdir_save_argv0)-1]=0;

  strncpy(findexecdir_save_path, getenv("PATH"), sizeof(findexecdir_save_path));
  findexecdir_save_path[sizeof(findexecdir_save_path)-1]=0;
  findexecdir_initialized=1;
}


int findexec(char *result, size_t size_of_result)
{

  char newpath[PATH_MAX+256];
  char newpath2[PATH_MAX+256];

  assert(findexecdir_initialized);
  result[0]=0;

  if(findexecdir_save_argv0[0]==findexecdir_path_separator) {
    if(findexecdir_debug) printf("  absolute path\n");
     realpath(findexecdir_save_argv0, newpath);
     if(findexecdir_debug) printf("  newpath=\"%s\"\n", newpath);
     if(!access(newpath, F_OK)) {
        strncpy(result, newpath, size_of_result);
        result[size_of_result-1]=0;
        return(0);
     } else {
    perror("access failed 1");
      }
  } else if( strchr(findexecdir_save_argv0, findexecdir_path_separator )) {
    if(findexecdir_debug) printf("  relative path to pwd\n");
    strncpy(newpath2, findexecdir_save_pwd, sizeof(newpath2));
    newpath2[sizeof(newpath2)-1]=0;
    strncat(newpath2, findexecdir_path_separator_as_string, sizeof(newpath2));
    newpath2[sizeof(newpath2)-1]=0;
    strncat(newpath2, findexecdir_save_argv0, sizeof(newpath2));
    newpath2[sizeof(newpath2)-1]=0;
    realpath(newpath2, newpath);
    if(findexecdir_debug) printf("  newpath=\"%s\"\n", newpath);
    if(!access(newpath, F_OK)) {
        strncpy(result, newpath, size_of_result);
        result[size_of_result-1]=0;
        return(0);
     } else {
    perror("access failed 2");
      }
  } else {
    if(findexecdir_debug) printf("  searching $PATH\n");
    char *saveptr;
    char *pathitem;
    for(pathitem=strtok_r(findexecdir_save_path, findexecdir_path_list_separator,  &saveptr); pathitem; pathitem=strtok_r(NULL, findexecdir_path_list_separator, &saveptr) ) {
       if(findexecdir_debug>=2) printf("pathitem=\"%s\"\n", pathitem);
       strncpy(newpath2, pathitem, sizeof(newpath2));
       newpath2[sizeof(newpath2)-1]=0;
       strncat(newpath2, findexecdir_path_separator_as_string, sizeof(newpath2));
       newpath2[sizeof(newpath2)-1]=0;
       strncat(newpath2, findexecdir_save_argv0, sizeof(newpath2));
       newpath2[sizeof(newpath2)-1]=0;
       realpath(newpath2, newpath);
       if(findexecdir_debug) printf("  newpath=\"%s\"\n", newpath);
      if(!access(newpath, F_OK)) {
          strncpy(result, newpath, size_of_result);
          result[size_of_result-1]=0;
          return(0);
      } 
    } // end for
    perror("access failed 3");

  } // end else
  // if we get here, we have tried all three methods on argv[0] and still haven't succeeded.   Include fallback methods here.
  return(1);
}

int findexecdir(char * argv0, char *result, size_t size_of_result) {
    findexecdir_init(argv0);
    char *tres = malloc(size_of_result);
    int r = findexec(tres, size_of_result);
    char *dname = dirname(tres);
    free(tres);
    strcpy(result, dname);
    free(dname);
    return r;   
}
