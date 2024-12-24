#include <defs.h>
#include <mmu.h>
#include <sem.h>
#include <ide.h>
#include <inode.h>
#include <kmalloc.h>
#include <dev.h>
#include <vfs.h>
#include <iobuf.h>
#include <error.h>
#include <assert.h>

#define DISK0_BLKSIZE                   PGSIZE
#define DISK0_BUFSIZE                   (4 * DISK0_BLKSIZE)
#define DISK0_BLK_NSECT                 (DISK0_BLKSIZE / SECTSIZE)

static char *disk0_buffer;
static semaphore_t disk0_sem;

static void
lock_disk0(void) {
    down(&(disk0_sem));
}

static void
unlock_disk0(void) {
    up(&(disk0_sem));
}

static int
disk0_open(struct device *dev, uint32_t open_flags) {
    return 0;
}

static int
disk0_close(struct device *dev) {
    return 0;
}

static void
disk0_read_blks_nolock(uint32_t blkno, uint32_t nblks) {
    int ret;
    uint32_t sectno = blkno * DISK0_BLK_NSECT, nsecs = nblks * DISK0_BLK_NSECT;
    if ((ret = ide_read_secs(DISK0_DEV_NO, sectno, disk0_buffer, nsecs)) != 0) {
        panic("disk0: read blkno = %d (sectno = %d), nblks = %d (nsecs = %d): 0x%08x.\n",
                blkno, sectno, nblks, nsecs, ret);
    }
}

static void
disk0_write_blks_nolock(uint32_t blkno, uint32_t nblks) {
    int ret;
    uint32_t sectno = blkno * DISK0_BLK_NSECT, nsecs = nblks * DISK0_BLK_NSECT;
    if ((ret = ide_write_secs(DISK0_DEV_NO, sectno, disk0_buffer, nsecs)) != 0) {
        panic("disk0: write blkno = %d (sectno = %d), nblks = %d (nsecs = %d): 0x%08x.\n",
                blkno, sectno, nblks, nsecs, ret);
    }
}

static int
disk0_io(struct device *dev, struct iobuf *iob, bool write) {
    off_t offset = iob->io_offset; // 从iobuf中获取I/O操作的起始偏移量
    size_t resid = iob->io_resid;  // 从iobuf中获取剩余数据量
    uint32_t blkno = offset / DISK0_BLKSIZE; // 计算起始块号
    uint32_t nblks = resid / DISK0_BLKSIZE; // 计算需要传输的块数量

    /* don't allow I/O that isn't block-aligned */
    if ((offset % DISK0_BLKSIZE) != 0 || (resid % DISK0_BLKSIZE) != 0) {
        return -E_INVAL;
    }

    /* don't allow I/O past the end of disk0 */
    if (blkno + nblks > dev->d_blocks) {
        return -E_INVAL;
    }

    /* read/write nothing ? */
    /* 如果没有数据需要读写，则直接返回 */
    if (nblks == 0) {
        return 0;
    }

    lock_disk0(); // 锁定磁盘，确保操作的原子性和一致性
    while (resid != 0) { // 当还有剩余数据需要处理时，继续循环
        size_t copied, alen = DISK0_BUFSIZE; // 当还有剩余数据需要处理时，继续循环
        if (write) { // 如果是写操作
            iobuf_move(iob, disk0_buffer, alen, 0, &copied); // 从iobuf移动数据到磁盘缓冲区
            assert(copied != 0 && copied <= resid && copied % DISK0_BLKSIZE == 0); // 确保复制的数据量有效
            nblks = copied / DISK0_BLKSIZE; // 计算需要写入的块数
            disk0_write_blks_nolock(blkno, nblks); // 写入数据到磁盘，不加锁版本
        }
        else { 
            if (alen > resid) { // 如果请求的数据长度大于剩余数据，则调整请求长度
                alen = resid;
            }
            nblks = alen / DISK0_BLKSIZE; // 计算需要读取的块数
            disk0_read_blks_nolock(blkno, nblks); // 从磁盘读取数据，不加锁版本
            iobuf_move(iob, disk0_buffer, alen, 1, &copied); // 从磁盘缓冲区移动数据到iobuf
            assert(copied == alen && copied % DISK0_BLKSIZE == 0); // 确保复制的数据量与请求的一致
        }
	// 减少剩余数据量  ; 更新块号
        resid -= copied, blkno += nblks;
    }
    unlock_disk0(); // 解锁磁盘
    return 0;  // 返回成功
}

static int
disk0_ioctl(struct device *dev, int op, void *data) {
    return -E_UNIMP;
}

static void
disk0_device_init(struct device *dev) {
    static_assert(DISK0_BLKSIZE % SECTSIZE == 0);
    if (!ide_device_valid(DISK0_DEV_NO)) {
        panic("disk0 device isn't available.\n");
    }
    dev->d_blocks = ide_device_size(DISK0_DEV_NO) / DISK0_BLK_NSECT;
    dev->d_blocksize = DISK0_BLKSIZE;
    dev->d_open = disk0_open;
    dev->d_close = disk0_close;
    dev->d_io = disk0_io;
    dev->d_ioctl = disk0_ioctl;
    sem_init(&(disk0_sem), 1);

    static_assert(DISK0_BUFSIZE % DISK0_BLKSIZE == 0);
    if ((disk0_buffer = kmalloc(DISK0_BUFSIZE)) == NULL) {
        panic("disk0 alloc buffer failed.\n");
    }
}

void
dev_init_disk0(void) {
    struct inode *node;
    if ((node = dev_create_inode()) == NULL) {
        panic("disk0: dev_create_node.\n");
    }
    disk0_device_init(vop_info(node, device));

    int ret;
    if ((ret = vfs_add_dev("disk0", node, 1)) != 0) {
        panic("disk0: vfs_add_dev: %e.\n", ret);
    }
}

