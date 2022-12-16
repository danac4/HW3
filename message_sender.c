//argv[1]= message slot file path, argv[2]= the target message slot file path, argv[3]=the message to pass
#include <sys/errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h> 

int main(int argc, char *argv[]){
    int fd;
    int channel_id;
    char *message; //should not include null of argv[3]

    if(argc != 4){
        perror("Invalid command line arguments in message_sender.c");
        exit(1);
    }
    fd = open(argv[1],ORDWR);
    if(fd == -1){
       perror("File failed to open in message_sender.c");
       exit(1);
    }
    //functionality
    close(fd); //success
    exit(0);
}