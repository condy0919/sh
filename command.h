#ifndef _COMMAND_INCLUDED
#define _COMMAND_INCLUDED

#define COMMAND_LENGTH 64

typedef struct command_t {
    char* cmd[COMMAND_LENGTH];
    int cmd_size;
} command_t;


static inline void init_command(command_t* p)
{
    p->cmd_size = 0;
}

#endif










