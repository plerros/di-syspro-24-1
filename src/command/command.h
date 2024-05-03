#ifndef SYSPROG24_1_COMMAND_H
#define SYSPROG24_1_COMMAND_H

enum command_types {cmd_invalid, cmd_issueJob, cmd_setConcurrency, cmd_stop, cmd_poll, cmd_exit};

int command_recognize(struct array *arr);

#endif /*SYSPROG24_1_COMMAND_H*/