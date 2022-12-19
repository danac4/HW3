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
    char buffer[BUFF_LEN];
    if(argc != 3){
        perror("Invalid command line arguments in message_reader.c");
        exit(1);
    }
    fd = open(argv[1], O_RDWR);
    if(fd == -1){
       perror("File failed to open in message_reader.c");
       exit(1);
    }
    /*https://www.systutorials.com/docs/linux/man/3-strtoul/, given non negative intager so no error check needed*/
    channel_id = strtoul(argv[2], NULL, 10);
    ret = ioctl(fd, MSG_SLOT_CHANNEL, channel_id);
    if(ret == -1){
        perror("icotl call failed in message_reader.c");
        exit(1);
    }
    ret = read(fd, buffer, BUFF_LEN);
    if(ret == -1){
        perror("read failed in message_reader.c");
        exit(1);
    }
    close(fd);
    if(write(1, buffer, ret) == -1){
        perror("faild to print message to standard output");
        exit(1);
    }
    exit(0);
}