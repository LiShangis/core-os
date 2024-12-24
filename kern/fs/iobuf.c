#include <defs.h>
#include <string.h>
#include <iobuf.h>
#include <error.h>
#include <assert.h>

struct iobuf *
iobuf_init(struct iobuf *iob, void *base, size_t len, off_t offset) {
    iob->io_base = base;
    iob->io_offset = offset;
    iob->io_len = iob->io_resid = len;
    return iob;
}

int
iobuf_move(struct iobuf *iob, void *data, size_t len, bool m2b, size_t *copiedp) {
    size_t alen; // 定义变量alen，用于存储实际要移动的数据长度
    // 计算实际可以移动的数据长度，不超过iobuf中剩余数据和请求移动的数据长度中较小的一个
    if ((alen = iob->io_resid) > len) {
        alen = len;
    }
// LAB9 EXERCISE1: YOUR CODE
    // 如果有数据需要移动（alen > 0）
    if (alen > 0) {
	    // 设置源地址和目标地址
	    void *src = iob->io_base, *dst = data;
	    // 根据m2b标志确定数据移动方向
	    if (m2b) {
		    // 如果m2b为true，交换源地址和目标地址，即数据从iobuf移动到data
		    void *tmp = src;
		    src = dst, dst = tmp;
	    } 
	    // 使用memmove函数移动数据
	    memmove(dst, src, alen);
	    // 更新iobuf的偏移量和剩余数据量
	    iobuf_skip(iob, alen), len -= alen;
    }
//You may need use the 'memmove()' and  'iobuf_skip()' functions

    // 如果提供了copiedp指针，则记录实际移动的数据长度
    if (copiedp != NULL) {
        *copiedp = alen;
    }

    // 如果len为0，表示数据已经全部移动，返回0；否则返回错误码-E_NO_MEM
    return (len == 0) ? 0 : -E_NO_MEM;
}

int
iobuf_move_zeros(struct iobuf *iob, size_t len, size_t *copiedp) {
    size_t alen;
    if ((alen = iob->io_resid) > len) {
        alen = len;
    }
    if (alen > 0) {
        memset(iob->io_base, 0, alen);
        iobuf_skip(iob, alen), len -= alen;
    }
    if (copiedp != NULL) {
        *copiedp = alen;
    }
    return (len == 0) ? 0 : -E_NO_MEM;
}

void
iobuf_skip(struct iobuf *iob, size_t n) {
    assert(iob->io_resid >= n);
    iob->io_base += n, iob->io_offset += n, iob->io_resid -= n;
}

