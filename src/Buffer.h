#ifndef BUFFER
#define BUFFER

class Buffer {
public:
    /* flags中标志位 */
	enum BufferFlag {
		B_WRITE = 0x1,	    /* 写操作。将缓存中的信息写到硬盘上去 */
		B_READ = 0x2,       /* 读操作。从盘读取信息到缓存中 */
		B_DONE = 0x4,       /* I/O操作结束 */
		B_ERROR = 0x8,      /* I/O因出错而终止 */
		B_BUSY = 0x10,	    /* 相应缓存正在使用中 */
		B_WANTED = 0x20,    /* 有进程正在等待使用该buf管理的资源，清B_BUSY标志时，要唤醒这种进程 */
		B_ASYNC = 0x40,		/* 异步I/O，不需要等待其结束 */
		B_DELWRI = 0x80		/* 延迟写，在相应缓存要移做他用时，再将其内容写到相应块设备上 */
	};

	unsigned int b_flags; // 缓存控制块标志位

	Buffer*	b_forw; // 前一个
	Buffer*	b_back; // 后一个

	int	b_wcount; // 需传送的字节数
	char* b_addr; // 指向该缓存控制块所管理的缓冲区的首地址
	int	b_blk_no; // 磁盘逻辑块号
	// int	b_error; // I/O出错时信息
	// int	b_resid; // I/O出错时尚未传送的剩余字节数

	Buffer();
	~Buffer();
};

#endif