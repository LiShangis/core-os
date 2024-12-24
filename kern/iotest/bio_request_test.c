#include <bio_request_test.h>
#include <io_scheduler.h>

extern struct io_scheduler default_scheduler;

// Create a new I/O request
static struct bio* create_bio(int blockNum) {
    if (blockNum < 0 || blockNum >= TOTAL_BLOCKS) {
        kprintf("Block number out of range: %d\n", blockNum);
        return NULL;
    }
    struct bio* newRequest = (struct bio*)kmalloc(sizeof(struct bio));
    if(newRequest == NULL){
        kprintf("Memory allocation failed\n");
        return NULL;
    }
    newRequest->block_num = blockNum;
    newRequest->next = NULL;
    newRequest->prev = NULL;

    // Allocate memory to store the read data
    void* base = (void*)kmalloc(512);
    struct iobuf __iob, *iob = iobuf_init(&__iob, base, 512, 0);
    newRequest->iob = *iob;

    return newRequest;
}

static int calculate_track_to_track_time(int current_block, int next_block) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
#endif
}

static int calculate_sector_to_sector_time(int current_block, int next_block) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
#endif
}

static int measure_time(int* traversed_blocks, int size, int begin_pos, int direction) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
    return 0;
#endif
}

void check_io_test() {
    default_scheduler.init_queue();

    int blockNums[] = {9824, 18553, 3744, 15522, 1466, 1224, 65, 676, 1111};
    int numRequests = sizeof(blockNums) / sizeof(blockNums[0]);

    kprintf("CYLINDERS = %d, HPC = %d, SPT = %d \n", CYLINDERS, HPC, SPT);
    kprintf("TRACK_SEEK_TIME = %d \n", TRACK_SEEK_TIME);
    kprintf("SECTOR_SEEK_TIME = %d \n", SECTOR_SEEK_TIME);
    kprintf("TIME_DATA_BLOCK_TRANSFER = %d \n", TIME_DATA_BLOCK_TRANSFER);
    kprintf("Request Sequence:\n");
    for (int i = 0; i < numRequests; i++) {
        kprintf("%d\t", blockNums[i]);
        struct bio *temp = create_bio(blockNums[i]);
        if (temp != NULL) {
            default_scheduler.submit_bio(temp);
        }
    }
    kprintf("\n");
    int begin_pos, direction;
    int *traversed_blocks;
    int size = 0;

    traversed_blocks = default_scheduler.schedule(&begin_pos, &direction, &size);

#ifdef LAB10_EX1
    int expected[] = {9824, 15522, 18553, 3744, 1466, 1224, 1111, 676, 65};
    int check_pass = 1;
    if (size != numRequests) {
        check_pass = 0;
    } else {
        for (int i = 0; i < numRequests; i++) {
            if (traversed_blocks[i] != expected[i]) {
                check_pass = 0;
                break;
            }
        }
    }
    if (check_pass) {
        kprintf("iotest check pass.\n");
    } else {
        kprintf("iotest check fail.\n");
    }
#endif
#ifdef LAB10_EX2
    kprintf("time = %d\n", measure_time(traversed_blocks, numRequests, begin_pos, direction));
#endif
    kfree(traversed_blocks);  // Free the traversed blocks array
}
