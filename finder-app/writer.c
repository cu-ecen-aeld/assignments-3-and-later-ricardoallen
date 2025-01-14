#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>

#define MAX_STRING_LONG _POSIX_PATH_MAX  

void my_exit(int x){
    closelog();
    exit (x);
}
int main(int argc, char **argv){
    char log_string[MAX_STRING_LONG] = "WRITER ";
    char cmd_name[MAX_STRING_LONG];
          
    char file_name[MAX_STRING_LONG];
    char input_str[MAX_STRING_LONG];


    openlog(log_string, LOG_PID, LOG_USER);
    if (NULL == strncpy(cmd_name,argv[0],MAX_STRING_LONG)){
        syslog(LOG_ERR, "[Error] Command Name too long '%s'",argv[0]);
        my_exit(5);

    }   
    syslog(LOG_INFO, "%s:[Info] Start writer",cmd_name);
    if (argc != 3){
        syslog(LOG_ERR, "%s:[Error] 2 arguments are requred: new_file string_to_write",cmd_name);
        my_exit(1);
    }
    if (NULL == strncpy(file_name,argv[1],MAX_STRING_LONG)){
        syslog(LOG_ERR, "%s:[Error] File path too long %s",cmd_name,argv[1]);
        my_exit(5);
    }
           
    if (NULL == strncpy(input_str,argv[2],MAX_STRING_LONG)){
        syslog(LOG_ERR, "%s:[Error] Input striing too long %s",cmd_name,argv[2]);
        my_exit(5);
    }
    FILE *new_file;
    new_file= fopen(file_name, "w");
    if(NULL == new_file){
        syslog(LOG_ERR, "%s:[Error] Creating %s, %s",cmd_name,file_name,strerror(errno));
        my_exit(1);
    }else{
        if(0 > fprintf(new_file, "%s", input_str)){
            syslog(LOG_ERR, "%s:[Error] writting %s to %s, %s",cmd_name,input_str,file_name,strerror(errno));
            my_exit(1);
        }else{
            syslog(LOG_DEBUG,"%s:[Debug]: \n Writing %s to %s'\n",cmd_name,input_str,file_name);
            fclose(new_file);
        }
    }
    my_exit (0);
}


