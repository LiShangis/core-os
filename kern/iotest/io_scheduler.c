#include <io_scheduler.h>
#include <kmalloc.h>

static struct bio* disk_head_look;
static struct bio* list_head_look;
static struct bio* list_tail_look;
static int direction_look;
static int request_count; // Maintain request count separately

// Insert a bio request into the scheduling queue, sorted by block number
static void submit_bio(struct bio* request) {
    if(request == NULL){
        return;
    }

    struct bio *p = list_head_look->next;

    while(p->block_num != -1 && request->block_num >= p->block_num) {
        p = p->next;
    }

    request->next = p;
    request->prev = p->prev;
    if(p->prev != NULL){
        p->prev->next = request;
    }
    p->prev = request;
    request_count++;
}

// Initialize the queue, disk_head_look represents the head position, list_head_look and list_tail_look represent the head and tail of the linked list
static int init_queue_look() {
    direction_look = INITIAL_DIRECTION;
    disk_head_look = (struct bio*)kmalloc(sizeof(struct bio));
    if(disk_head_look == NULL){
        kprintf("Memory allocation failed for disk_head_look\n");
        return -1;
    }
    disk_head_look->block_num = INITIAL_HEAD_POS;
    request_count = 0;

    list_head_look = (struct bio*)kmalloc(sizeof(struct bio));
    if(list_head_look == NULL){
        kprintf("Memory allocation failed for list_head_look\n");
        kfree(disk_head_look);
        return -1;
    }
    list_head_look->block_num = -1;

    list_tail_look = (struct bio*)kmalloc(sizeof(struct bio));
    if(list_tail_look == NULL){
        kprintf("Memory allocation failed for list_tail_look\n");
        kfree(disk_head_look);
        kfree(list_head_look);
        return -1;
    }
    list_tail_look->block_num = -1;

    disk_head_look->next = list_tail_look;
    disk_head_look->prev = list_head_look;
    list_head_look->next = disk_head_look;
    list_head_look->prev = NULL;
    list_tail_look->next = NULL;
    list_tail_look->prev = disk_head_look;

    return 0;
}

int* look_schedule(int *begin_pos, int *begin_direction, int *size) {
    struct bio* current; // 用于遍历链表的当前指针
    int* traversed_blocks = (int*)kmalloc(request_count * sizeof(int)); // 分配内存存储遍历的块号
    int count = 0; // 已遍历的块号计数器

    *begin_pos = disk_head_look->block_num; // 设置磁头起始位置
    *begin_direction = direction_look; // 设置初始遍历方向
    kprintf("begin_pos = %d, begin_direction = %d\n", *begin_pos, *begin_direction);
#ifdef LAB10_EX1 
    // LAB10 EXERCISE1: YOUR CODE

    while (1) { // 无限循环，直到遍历完成所有请求
	    current = direction_look ? disk_head_look->next : disk_head_look->prev; // 根据方向选择起始节点
	    while (current->block_num != -1) { // 遍历直到遇到结束标记
		    traversed_blocks[count++] = current->block_num; // 记录当前块号
		    (*size)++; // 更新遍历的总块号数
		    disk_head_look->block_num = current->block_num; // 更新磁头位置
		    if (direction_look) { // 如果当前方向为正（向前）
			    disk_head_look->next = current->next; // 更新磁头的下一个节点
			    if (current->next != NULL) { // 如果下一个节点存在，更新其前一个节点
				    current->next->prev = disk_head_look;
			    }
		    } else { // 如果当前方向为负（向后）
			    disk_head_look->prev = current->prev; // 更新磁头的上一个节点
			    if (current->prev != NULL) { // 如果上一个节点存在，更新其下一个节点
				    current->prev->next = disk_head_look;
			    }
		    }
		    struct bio* temp = current; // 临时保存当前节点，以便释放
		    current = direction_look ? current->next : current->prev; // 移动到下一个节点
		    kfree(temp->iob.io_base); // 释放iob的内存
		    kfree(temp); // 释放当前节点的内存
	    }
	    // Change direction
	    direction_look = !direction_look; // 切换遍历方向
	    // Check if there are requests in the other direction
	    if ((direction_look ? disk_head_look->next->block_num : 
				    disk_head_look->prev->block_num) == -1) {
		    break; // 如果没有请求，退出循环
	    }
    }
#endif
    request_count = 0;
    return traversed_blocks;
}

struct io_scheduler default_scheduler = {
    .init_queue = init_queue_look,
    .submit_bio = submit_bio,
    .schedule = look_schedule,
};
