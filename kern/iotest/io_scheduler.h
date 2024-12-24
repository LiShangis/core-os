#ifndef IO_SCHEDULER_H
#define IO_SCHEDULER_H

#include <iobuf.h>

#define INITIAL_HEAD_POS 8000
#define INITIAL_DIRECTION 1

#define CYLINDERS 200
#define HPC 100
#define SPT 50
#define TOTAL_BLOCKS (CYLINDERS * HPC * SPT)

struct bio {
    struct bio *next;
    struct bio *prev;
    struct iobuf iob;   // buffer
    int block_num;      // logical block number
};

struct io_scheduler {
    int (*init_queue)();
    void (*submit_bio)(struct bio* request);
    int* (*schedule)(int *begin_pos, int *begin_direction, int *size);
};

extern struct io_scheduler default_scheduler;

#endif // IO_SCHEDULER_H