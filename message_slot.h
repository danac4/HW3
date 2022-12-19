#define MAJOR 235
#define BUFF_LEN 128

#define MSG_SLOT_CHANNEL _IOW(MAJOR, 0, unsigned long)

typedef struct Channel{
    unsigned int id;
    char *last_message;
    int message_len;
    struct Channel *next;
}Channel;
