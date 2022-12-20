#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define MAJOR_NUM 235//changed to 255 for testing
#define BUFF_LEN 128

#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)
#define DEVICE_RANGE_NAME "message_slot_dev"

typedef struct ChannelStruct{
    unsigned int id;
    char *last_message;
    int message_len;
    struct ChannelStruct *next;
}Channel;

#endif