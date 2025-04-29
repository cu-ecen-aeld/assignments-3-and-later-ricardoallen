#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <stdlib.h>
#include <syslog.h>
#include <string.h>

#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define MAX_STRING_LONG _POSIX_PATH_MAX  
#define AES_FILE "/var/tmp/aesdsocketdata"
#define SRV_RCV_BUFFER_SIZE 512
char buffer[SRV_RCV_BUFFER_SIZE];   
int signal_exit = 0;
char log_string[MAX_STRING_LONG] = "AESDSOCKET ";
char cmd_name[MAX_STRING_LONG];
int aes_socket, accept_fd;
int getaddrinfo_result;
int bind_result;
struct addrinfo hints;
struct addrinfo *servinfo;
socklen_t addr_size;
struct sockaddr_storage aessockaddr;

static void aes_signal_handler(int aes_signal_number){
    if((aes_signal_number == SIGINT) || (aes_signal_number == SIGTERM)){
        signal_exit=1;
    }
}

void my_exit(int x){
    closelog();
    exit (x);
}
int main(int argc, char **argv){
    int deamon_mode = 0;
    if(argc>0){
        for(int i=1;i<argc;i++){
            printf("arg%d '%s'\n",i,argv[i]);
            if(strcmp("-d",argv[i])==0){
                deamon_mode=1;
            }
        }
    }
    printf("deamon mode = %d\n",deamon_mode);
    openlog(log_string, LOG_PID, LOG_USER);
    if (NULL == strncpy(cmd_name,argv[0],MAX_STRING_LONG)){
        syslog(LOG_ERR, "[Error] Command Name too long '%s'",argv[0]);
        my_exit(-1);
    }
    //  from https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    
    getaddrinfo_result = getaddrinfo(NULL, "9000", &hints, &servinfo);
    if (getaddrinfo_result != 0){
        syslog(LOG_ERR, "%s:[Error] getaddrinfo errorsocket creation failed: '%s'",cmd_name,gai_strerror(errno));
        my_exit(-1);
    }
    aes_socket  = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (aes_socket == -1){
        syslog(LOG_ERR, "%s:[Error] socket creation failed: '%m'",cmd_name);
        my_exit(-1);
    }
    int so_reuseaddre_enable =1;
    if(setsockopt(aes_socket,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddre_enable,sizeof(int)) == -1){
        syslog(LOG_ERR, "%s:[Error] setsockipt failed: '%m'",cmd_name);
        my_exit(-1);
    }
    
    bind_result = bind(aes_socket, servinfo->ai_addr, servinfo->ai_addrlen);
     if (bind_result != 0){
        syslog(LOG_ERR, "%s:[Error] bind failed: '%m'",cmd_name);
        my_exit(-1);
    }
    freeaddrinfo(servinfo);
    // Reading from and writing to the socket
    ssize_t recv_size;
    int data_complete;
    size_t msg_size;
    size_t msg_end;
    char *msg_buff = NULL;
    char *new_buff = NULL;
    int done;
    if(deamon_mode){
       pid_t pid =fork();
       if(pid < 0){
           syslog(LOG_ERR, "%s:[Error] fork failed: '%m'",cmd_name);
           my_exit(-1);
       }else if(pid > 0){
           printf("Socket binded succesfully deamon running as %x\n",pid);
           sleep(2);
           my_exit(0);
       }else{
           int sid = setsid();
           if(sid < 0){
               syslog(LOG_ERR, "%s:[Error] setsid failed: '%m'",cmd_name);
               my_exit(-1);
           }
           pid =fork();
           if(pid ==-1){
               syslog(LOG_ERR, "%s:[Error] fork failed: '%m'",cmd_name);
               my_exit(-1);
           }else if(pid >0){
               printf("Detaching terminal for deamon %x\n",pid);
               sleep(1);
               my_exit(0);
           }else{
               chdir("/");
               umask(0);
               close(STDIN_FILENO);
               close(STDOUT_FILENO);
               close(STDERR_FILENO);
               openlog(log_string, LOG_PID, LOG_USER);
           }
       }
    }
    struct sigaction aes_signal_action;
    memset(&aes_signal_action,0,sizeof(struct sigaction));
    aes_signal_action.sa_handler=aes_signal_handler;

    if(sigaction(SIGTERM, &aes_signal_action, NULL)){
        syslog(LOG_ERR, "%s:[Error] sigaction SIGTERM failed: '%m'",cmd_name);
        my_exit(-1);
    }
    if(sigaction(SIGINT, &aes_signal_action, NULL)){
        syslog(LOG_ERR, "%s:[Error] sigaction SIGINT failed: '%m'",cmd_name);
        my_exit(-1);
    }

    // Listening for incoming connections
    if (listen(aes_socket, 8) < 0) {
        syslog(LOG_ERR, "%s:[Error] listen failed: '%m'",cmd_name);
        my_exit(-1);
    }
    syslog(LOG_INFO,"%s:[Info] aes_socket listening on port %d\n", cmd_name,9000);
        

    while(!signal_exit){
        if(msg_end != 0){
            free(msg_buff);
            msg_buff=NULL;
            syslog(LOG_INFO,"%s:[Info] free memory %p\n", cmd_name,msg_buff);
            msg_size=0;
            msg_end=0;
        }
        buffer[0] = '\0';   
        data_complete=0;
        done = 0;

        // Accepting a connection
        addr_size= sizeof(aessockaddr);
    
    
        if ((accept_fd = accept(aes_socket, (struct sockaddr *)&aessockaddr, &addr_size)) < 0) {
            if ((errno == EINTR) && signal_exit){
                break;
            }else{
                syslog(LOG_ERR, "%s:[Error] socket accept failed: '%m'",cmd_name);
                my_exit(-1);
            }
                
        }
        if (getpeername(accept_fd, (struct sockaddr *)&aessockaddr, &addr_size) < 0) {
            syslog(LOG_ERR, "%s:[Error] getpeername: '%m'",cmd_name);
            my_exit(-1);
        }
        // Convert the IP address to string
        char ip_address[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, (struct sockaddr *)&aessockaddr, ip_address, INET_ADDRSTRLEN) == NULL) {
            syslog(LOG_ERR, "%s:[Error] getpeername: '%m'",cmd_name);
            my_exit(-1);
        }

        syslog(LOG_INFO,"%s:[Info] aes_socket Accepted connection from %s\n", cmd_name,ip_address);  
        do{
            recv_size = recv(accept_fd, buffer, SRV_RCV_BUFFER_SIZE-1,0);
            //printf("reading buffer size %zu '%s'\n",recv_size,buffer);
            if(recv_size < 0){
               syslog(LOG_ERR, "%s:[Error] recv failed: '%m'",cmd_name);
               my_exit(-1);
            }else if(recv_size==0){
               syslog(LOG_INFO,"%s:[Info] reading buffer size 0",cmd_name);
               done=1;
            }else{
                buffer[recv_size]='\0';
               
                //printf("reading buffer size %zu '%s'\n",recv_size,buffer);
                char *new_line = strchr(buffer,'\n');
                if(new_line != NULL){
                    done=1;
                    data_complete=1;
                }

                if(recv_size>=(msg_size-msg_end)){
                   if(data_complete){
                        msg_size=recv_size+msg_end+1;
                   }else if(msg_size ==0){
                        msg_size=1024;
                   }else msg_size*=2;
                }
                new_buff = realloc(msg_buff,msg_size);
                if (new_buff == NULL){
                    syslog(LOG_ERR, "%s:[Error] msg_size: '%zu' realloc failed: '%m'",cmd_name,msg_size);
                    my_exit(-1);
                }
                syslog(LOG_INFO,"%s:[Info] realloc memory old %p new %p\n", cmd_name,msg_buff,new_buff);
                msg_buff = new_buff;
                
                msg_buff[msg_end]='\0';
                strcat(msg_buff,buffer);
                msg_end = recv_size + msg_end ;
                
            }
            syslog(LOG_INFO,"%s:[Info] done '%d', data_complete '%d', msg_size '%zu', msg_end '%lu', recv_size '%zu'",cmd_name,
                              done,data_complete,msg_size,msg_end,recv_size);    
	}while(done == 0 && signal_exit ==0);
        if(data_complete==1){
            FILE *srv_fd;
            srv_fd = fopen(AES_FILE,"a");
            fprintf(srv_fd,"%s",msg_buff);
            fclose(srv_fd);

            struct stat srv_fd_stat;
            int srv_fd_read = open(AES_FILE,O_RDONLY);
            off_t srv_file_size;
            off_t srv_offset=0;
            if(fstat(srv_fd_read,&srv_fd_stat)<0){
                syslog(LOG_ERR, "%s:[Error] fstat failed: '%m'",cmd_name);
                my_exit(-1);
                  
            }
            srv_file_size = srv_fd_stat.st_size;
            ssize_t send_file_size = sendfile(accept_fd,srv_fd_read,&srv_offset,srv_file_size);
            if(send_file_size < 0){
                syslog(LOG_ERR, "%s:[Error] sendfile failed: '%m'",cmd_name);
                my_exit(-1);
            }
            syslog(LOG_INFO,"%s:[Info] file size %zu send size %zu",cmd_name,srv_file_size,send_file_size);
            close(srv_fd_read);
        } 
        // Closing the socket
        close(accept_fd);
        syslog(LOG_INFO,"%s:[Info] aes_socket Closed connection from %s\n", cmd_name,ip_address);
    }
    if(msg_size != 0){
        free(msg_buff);
        msg_buff=NULL;
        msg_size=0;
    }
    syslog(LOG_INFO,"%s:[Info] aes_socket Caught signal, exiting\n", cmd_name); 
    unlink(AES_FILE);
    close(aes_socket);
    
    my_exit (0);
}


