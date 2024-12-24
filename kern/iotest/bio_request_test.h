#ifndef BIO_REQUEST_FS_H
#define BIO_REQUEST_FS_H

void check_io_test();

#define CYLINDERS 200
#define HPC 100
#define SPT 50

#define TRACK_SEEK_TIME 2 // Track seek time in milliseconds
#define SECTOR_SEEK_TIME 1 // Sector seek time in milliseconds
#define TIME_DATA_BLOCK_TRANSFER 4 // Time to transfer a block of data in milliseconds

#define ABS(x) ((x) < 0 ? -(x) : (x))

#endif // BIO_REQUEST_FS_H