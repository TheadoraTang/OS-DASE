#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define TIMES 800  // 读写次数
#define maxline (1024 * 1024) // 字符串缓冲区buff的最大长度
#define filesize (300 * 1024 * 1024) // 文件的总大小
#define buffsize (1024 * 1024 * 1024) // 读写缓冲区rbuff的大小（1GB）
char rbuff[buffsize];
char *filepathDisk[15] = {"/usr/1.txt", "/usr/2.txt", "/usr/3.txt", "/usr/4.txt", "/usr/5.txt", "/usr/6.txt", "/usr/7.txt", "/usr/8.txt", "/usr/9.txt", "/usr/10.txt", "/usr/11.txt", "/usr/12.txt", "/usr/13.txt", "/usr/14.txt", "/usr/15.txt"};
char *filepathRam[15] = {"/root/myram/1.txt", "/root/myram/2.txt", "/root/myram/3.txt", "/root/myram/4.txt", "/root/myram/5.txt", "/root/myram/6.txt", "/root/myram/7.txt", "/root/myram/8.txt", "/root/myram/9.txt", "/root/myram/10.txt", "/root/myram/11.txt", "/root/myram/12.txt", "/root/myram/13.txt", "/root/myram/14.txt", "/root/myram/15.txt"};
char buff[maxline] = "RAMDISK"; // 存储在内存中的数据块，用于进行文件写操作

void write_file(int blocksize, bool isrand, char *filepath) {
    int fd = open(filepath, O_RDWR | O_CREAT | O_SYNC, 0755);
    if (fd < 0) {
        printf("Open file error!");
        return;
    }
    int temp;  // 记录实际写入
    // 多次重复写入计算时间
    for (int i = 0; i < TIMES;) {
        if ((temp = write(fd, buff, blocksize)) != blocksize) { // 检查字节数是否等于期待的块大小
            printf("Write error!\n");
        }
        if (isrand)
        {
            lseek(fd, rand() % filesize , SEEK_SET);  // 利用随机函数写到文件的任意一个位置
        }
            
        if (temp == blocksize)
        {
            i++;
        }
    }
    // 如果读到末尾则从文件开头开始读。
    lseek(fd, 0, SEEK_SET);  // 重设文件指针
    close(fd);  // 关闭文件描述符
}

void read_file(int blocksize, bool isrand, char *filepath) {
    int fd = open(filepath, O_RDWR | O_CREAT | O_SYNC, 0755);
    if (fd <= 0) {
        printf("Open error!");
        return;
    }
    int temp;  // 记录实际读取
    // 多次重复读取计算时间
    for (int i = 0; i < TIMES; ) {
        // 通过read函数从文件描述符fd中读取数据到缓冲区rbuff，读取的字节数保存在变量temp中
        if ((temp = read(fd, rbuff, blocksize)) != blocksize) {
            printf("Read file error!\n");
        }
        if (isrand)
            lseek(fd, rand() % filesize, SEEK_SET);  // 利用随机函数读取文件的任意一个位置
        if(temp == blocksize)
        {
            i++;
        }
    }
    // 如果读到末尾则从文件开头开始读。
    lseek(fd, 0, SEEK_SET);  
    close(fd);  // 关闭文件描述符
}

long get_time(struct timeval starttime, struct timeval endtime) {
    long spendtime;
    spendtime = (long)(endtime.tv_sec - starttime.tv_sec) * 1000 + (endtime.tv_usec - starttime.tv_usec) / 1000; //tv_sec为创建struct timeval时的秒数，tv_usec为微秒数，即秒后面的零头
    // 换算成毫秒
    return spendtime;
}

int main() {
    srand((unsigned)time(NULL));
    struct timeval starttime, endtime;
    double spendtime;
    for (int i = 0; i < maxline; i += 16) {
        strcat(buff, "1111111111111111");
    }
    for (int blocksize = 64; blocksize <= 1024 * 64; blocksize *= 4) {
        int Concurrency = 12;//7-15个进程达到饱和，根据实际情况不断调整
        gettime(&starttime, NULL);
        for (int i = 0; i < Concurrency; i++) {
            if (fork() == 0) {
                //随机写
                //write_file(blocksize,true,filepathDisk[i]);
                //write_file(blocksize,true,filepathRam[i]);
                //顺序写
                //write_file(blocksize,false,filepathDisk[i]);
                //write_file(blocksize,false,filepathRam[i]);
                //随机读
                //read_file(blocksize,true,filepathDisk[i]);
                //read_file(blocksize,true,filepathRam[i]);
                //顺序读
                read_file(blocksize,false,filepathDisk[i]);
                //read_file(blocksize,false,filepathRam[i]);
                exit(0);
            }
        }
        // 当所有子进程结束
        while (wait(NULL) != -1);
        gettime(&endtime, NULL);
        spendtime = get_time(starttime, endtime) / 1000.0;
        double eachtime = spendtime / TIMES;//平均时间
        double block = blocksize * Concurrency / 1024.0 / 1024.0;
        printf("blocksize=%.4fKB, speed=%fMB/s\n", (double)blocksize / 1024.0, block / eachtime);
    }
    return 0;
}

