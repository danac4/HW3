#include "message_slot.h"

#include <sys/errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    int fd, ret;
    unsigned long channel_id;

    if(argc != 4){
        perror("Invalid command line arguments in message_sender.c");
        exit(1);
    }
    fd = open(argv[1],O_RDWR);
    if(fd == -1){
       perror("File failed to open in message_sender.c");
       exit(1);
    }
    /*https://www.systutorials.com/docs/linux/man/3-strtoul/, given non negative intager so no error check needed*/
    channel_id = strtoul(argv[2],NULL,10);
    ret = ioctl(fd,MSG_SLOT_CHANNEL, channel_id);
    if(ret == -1){
        perror("icotl call failed in message_sender.c");
        exit(1);
    }
    ret = write(fd, argv[3],strlen(argv[3]));
    if(ret == -1){
        perror("write call failed in message_sender.c");
        exit(1);
    }
    close(fd); //success
    exit(0);
}