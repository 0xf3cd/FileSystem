#include "INode.h"
#include "BufferManager.h"
#include "FileSystem.h"
#include "File.h"
#include <iostream>
#include <string.h>
#include <math.h>

extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;

File::File(MemINode* minode) {
    BM = &g_BufferManager;
    FS = &g_FileSystem;

    f_type = 0;
    if(minode -> m_mode & MemINode::IFDIR) {
        f_type |= File::FDIR;
    }
    f_minode = minode;
    f_offset = 0;
}

File::~File() {
    // inode 不在此释放
    // 由使用这个 File 对象的使用者释放
}

/**
 * 获取文件大小
 */
int File::getFileSize() {
    return f_minode -> m_size;
}

/**
 * 获取文件使用的盘块数
 */
int File::getBlkNum() {
    const int fsize = f_minode -> m_size;
    int blk_num = fsize / 512;
    if(fsize % 512 != 0) {
        blk_num++;
    }
    return blk_num;
}

/**
 * 得到当前文件使用的最后一块盘块剩余容量
 */
int File::getLastBlkRest() {
    const int fsize = f_minode -> m_size;
    return fsize % 512;
}

/**
 * 为文件新申请一个盘块
 * 返回相应缓存块
 */
Buffer* File::applyNewBlk() {
    const int pre_blk_num = getBlkNum();
    if(pre_blk_num >= HUGE_FILE_MOST_BLK) {
        cout << "文件过大！" << endl;
        return nullptr;
    }

    f_minode -> m_mode |= MemINode::IUPD; // 需要更新 INode

    Buffer* new_buf = FS -> allocBlock(); // 新分配的盘块
    const int new_blk_no = new_buf -> b_blk_no;
    int* addr = f_minode -> m_addr;

    // pre_blk_num 为申请新块之前的块数
    if(pre_blk_num < SMALL_FILE_MOST_BLK) { // 0 - 5 个盘块，不需要索引
        addr[pre_blk_num] = new_blk_no;
    } else if(pre_blk_num < LARGE_FILE_MOST_BLK) { // （6 - 6 + 128 * 2） 个盘块，一次间接索引
        const int indexed_blk_num = pre_blk_num + 1 - 6; // 除去前六个盘块，需要进行索引的盘块数
        if(indexed_blk_num % 128 == 1) {
            // 如果在某个索引块未被创建
            Buffer* new_index_blk = FS -> allocBlock();
            const int new_index_blk_no = new_index_blk -> b_blk_no;

            if(indexed_blk_num > 128) {
                addr[7] = new_index_blk_no;
            } else {
                addr[6] = new_index_blk_no;
            }

            // cout << "!" << new_index_blk -> b_blk_no << endl;
            BM -> clearBuf(new_index_blk);
            BM -> freeBuf(new_index_blk); // 至此，新的索引块已经分配完毕
        }

        const int inner_index_blk_no = 1 + (indexed_blk_num - 1) % 128; // 新的块在块内的编号
        Buffer* index_blk; // 需要更新的索引块
        if(indexed_blk_num > 128) {
            index_blk = BM -> readBuf(addr[7]);
        } else {
            index_blk = BM -> readBuf(addr[6]);
        } 
        // 现在需要将新分配的块号写入索引块
        int* write_addr = &(((int*)(index_blk -> b_addr))[inner_index_blk_no-1]); // 需要写入的首地址
        *write_addr = new_blk_no;
        BM -> dwriteBuf(index_blk); // 将修改后的索引块写回
    } else if(pre_blk_num < HUGE_FILE_MOST_BLK){ // 需要二次间接索引
        const int indexed_blk_num = pre_blk_num + 1 - 6 - 128 * 2; // 除去前面不需要索引及一级索引的盘块，需要进行索引的盘块数
        if(indexed_blk_num % (128 * 128) == 1) {
            // 如果一级索引块未被创建
            Buffer* new_index1_blk = FS -> allocBlock();
            const int new_index1_blk_no = new_index1_blk -> b_blk_no;

            if(indexed_blk_num > 128*128) {
                addr[9] = new_index1_blk_no;
            } else {
                addr[8] = new_index1_blk_no;
            }

            // cout << "!" << new_index1_blk -> b_blk_no << endl;
            BM -> clearBuf(new_index1_blk);
            BM -> freeBuf(new_index1_blk); // 至此，新的一级索引块已经分配完毕
        }

        if(indexed_blk_num % 128 == 1) {
            // 如果二级索引块未被创建
            Buffer* new_index2_blk = FS -> allocBlock();
            const int new_index2_blk_no = new_index2_blk -> b_blk_no;

            Buffer* index1_blk; // 一级索引块
            if(indexed_blk_num > 128*128) {
                index1_blk = BM -> readBuf(addr[9]);
            } else {
                index1_blk = BM -> readBuf(addr[8]);
            }

            const int inner_index1_blk_no = 1 + ((indexed_blk_num - 1) / 128) % 128; // 新的二级索引块块在第一级索引块内的编号
             // 现在需要将新分配二级索引块号写入一级索引块
            int* write_addr = &(((int*)(index1_blk -> b_addr))[inner_index1_blk_no-1]); // 需要写入的首地址
            *write_addr = new_index2_blk_no;

            // cout << "!!" << new_index2_blk -> b_blk_no << endl;
            BM -> dwriteBuf(index1_blk); // 写回更新后的一级索引块
            BM -> clearBuf(new_index2_blk);
            BM -> freeBuf(new_index2_blk); // 至此，新的二级索引块已经分配完毕
        }

        Buffer* index1_blk;
        Buffer* index2_blk;
        if(indexed_blk_num > 128*128) {
            index1_blk = BM -> readBuf(addr[9]);
        } else {
            index1_blk = BM -> readBuf(addr[8]);
        }

        const int inner_index1_blk_no = 1 + ((indexed_blk_num - 1) / 128) % 128; // 二级索引块在第一级索引块内的编号
        const int index2_blk_no = ((int*)(index1_blk -> b_addr))[inner_index1_blk_no-1]; // 找出二级索引块的物理盘块编号
        index2_blk = BM -> readBuf(index2_blk_no);

        const int inner_index2_blk_no = 1 + (indexed_blk_num - 1) % 128; // 新分配的盘块在第二级索引块内的编号
        int* write_addr = &(((int*)(index2_blk -> b_addr))[inner_index2_blk_no-1]); // 需要写入的首地址
        *write_addr = new_blk_no;

        BM -> freeBuf(index1_blk);
        BM -> dwriteBuf(index2_blk);
    } else {
        cout << "错误，检查 File.cc applyNewBlk 函数" << endl;
        return nullptr;
    }

    // f_minode -> m_size += 512;
    // cout << new_buf -> b_blk_no << endl;
    return new_buf;
}

/**
 * 进行逻辑盘块号到物理盘块号的映射
 */
int File::mapBlk(int lbn) {
    int bn;
    const int* addr = f_minode -> m_addr;
    const int fsize = f_minode -> m_size;
    const int blk_num = getBlkNum();

    if(lbn > blk_num || lbn < 0) {
        cout << "文件没有第 " << lbn << " 个逻辑块" << endl;
        return -1;
    }

    if(lbn < SMALL_FILE_MOST_BLK) { // 0 - 5 个盘块，不需要索引
        return addr[lbn];
    } else if(lbn < LARGE_FILE_MOST_BLK) { // （6 - 6 + 128 * 2） 个盘块，一次间接索引
        const int indexed_blk_num = lbn + 1 - 6; // 除去前六个盘块的盘块数
        Buffer* index_blk; // 一级索引块
        if(indexed_blk_num > 128) {
            index_blk = BM -> readBuf(addr[7]);
        } else {
            index_blk = BM -> readBuf(addr[6]);
        } 

        const int inner_index_blk_no = 1 + (indexed_blk_num - 1) % 128; // lbn 对应的物理盘块号储存在一级索引块中的位置
        bn = ((int*)(index_blk -> b_addr))[inner_index_blk_no-1];
        BM -> freeBuf(index_blk);
    } else if(lbn < HUGE_FILE_MOST_BLK){ // 需要二次间接索引
        const int indexed_blk_num = lbn + 1 - 6 - 128 * 2; // 除去前面不需要索引及一级索引的盘块，需要进行索引的盘块数
        Buffer* index1_blk;
        Buffer* index2_blk;
        if(indexed_blk_num > 128*128) {
            index1_blk = BM -> readBuf(addr[9]);
        } else {
            index1_blk = BM -> readBuf(addr[8]);
        }

        const int inner_index1_blk_no = 1 + ((indexed_blk_num - 1) / 128) % 128; // 二级索引块在第一级索引块内的编号
        const int index2_blk_no = ((int*)(index1_blk -> b_addr))[inner_index1_blk_no-1]; // 找出二级索引块的物理盘块编号
        index2_blk = BM -> readBuf(index2_blk_no);
        const int inner_index2_blk_no = 1 + (indexed_blk_num - 1) % 128; // lbn 号盘块在第二级索引块内的编号
        bn = ((int*)(index2_blk -> b_addr))[inner_index2_blk_no-1];

        BM -> freeBuf(index1_blk);
        BM -> freeBuf(index2_blk);
    } else {
        cout << "文件过大，" << "检查 File.cc 中的 mapBlk 函数" << endl;
        return -1;
    }

    return bn;
}

/**
 * 删除文件的所有内容
 */
void File::deleteAll() {
    f_minode -> m_mode |= MemINode::IUPD;
    const int blk_num = getBlkNum();
    int i, j, k;

    for(i = 0; i < blk_num; i++) {
        int blk_to_free = mapBlk(i);
        FS -> freeBlock(blk_to_free);
    }

    // 接下来清除索引节点
    const int* addr = f_minode -> m_addr;
    if(addr[6] != 0) {
        Buffer* index1_blk = BM -> readBuf(addr[6]);
        const int* indexed_blk = (int*)(index1_blk -> b_addr);
        for(j = 0; j < 512 / sizeof(int); j++) {
            if(indexed_blk[j] == 0) {
                break;
            }
            FS -> freeBlock(indexed_blk[j]);
        }

        BM -> freeBuf(index1_blk);
        FS -> freeBlock(addr[6]);
    }

    if(addr[7] != 0) {
        Buffer* index1_blk = BM -> readBuf(addr[7]);
        const int* indexed_blk = (int*)(index1_blk -> b_addr);
        for(j = 0; j < 512 / sizeof(int); j++) {
            if(indexed_blk[j] == 0) {
                break;
            }
            FS -> freeBlock(indexed_blk[j]);
        }

        BM -> freeBuf(index1_blk);
        FS -> freeBlock(addr[7]);
    }

    if(addr[8] != 0) {
        Buffer* index1_blk = BM -> readBuf(addr[8]);
        const int* indexed1_blk = (int*)(index1_blk -> b_addr);
        for(j = 0; j < 512 / sizeof(int); j++) {
            if(indexed1_blk[j] == 0) {
                break;
            }

            Buffer* index2_blk = BM -> readBuf(indexed1_blk[j]);
            const int* indexed2_blk = (int*)(index2_blk -> b_addr);
            for(k = 0; k < 512 / sizeof(int); k++) {
                if(indexed2_blk[j] == 0) {
                    break;
                }
                FS -> freeBlock(indexed2_blk[k]);
            }

            BM -> freeBuf(index2_blk);
            FS -> freeBlock(indexed1_blk[j]);
        }

        BM -> freeBuf(index1_blk);
        FS -> freeBlock(addr[8]);
    }

    if(addr[9] != 0) {
        Buffer* index1_blk = BM -> readBuf(addr[9]);
        const int* indexed1_blk = (int*)(index1_blk -> b_addr);
        for(j = 0; j < 512 / sizeof(int); j++) {
            if(indexed1_blk[j] == 0) {
                break;
            }

            Buffer* index2_blk = BM -> readBuf(indexed1_blk[j]);
            const int* indexed2_blk = (int*)(index2_blk -> b_addr);
            for(k = 0; k < 512 / sizeof(int); k++) {
                if(indexed2_blk[j] == 0) {
                    break;
                }
                FS -> freeBlock(indexed2_blk[k]);
            }

            BM -> freeBuf(index2_blk);
            FS -> freeBlock(indexed1_blk[j]);
        }

        BM -> freeBuf(index1_blk);
        FS -> freeBlock(addr[9]);
    }
}

/**
 * 读取文件内容
 * 以当前 f_offset 为起始字节开始读取
 * 返回读取的字节数
 */
int File::read(char* content, int length) {
    const int rest_byte_num = max(getFileSize() - f_offset, 0); // 文件剩余的大小
    const int toread_byte_num = min(rest_byte_num, length);

    int i;
    int read_byte_count = 0;
    int has_read_blk_num = f_offset / 512; // 已经读过的块数（逻辑块），根据 f_offset 确定
    int blk_rest_byte_num = 512 - (f_offset - has_read_blk_num * 512); // 在当前盘块内，当前文件指针后，还剩余多少字节未读取

    while(read_byte_count < toread_byte_num) {
        if(blk_rest_byte_num == 0) {
            blk_rest_byte_num = 512;
            has_read_blk_num++;
        }

        const int rest_toread_byte_num = toread_byte_num - read_byte_count; // 剩余待读取字节数
        if(blk_rest_byte_num >= rest_toread_byte_num) { // 如果当前块剩余字节数大于剩余所需读取字节数
            // 只需要读取本块，便可以完成本轮读取
            const int bn = mapBlk(has_read_blk_num);
            Buffer* buf = BM -> readBuf(bn);
            const char* addr = buf -> b_addr;

            // cout << "读取 " << has_read_blk_num << " 块盘块，" << rest_toread_byte_num << " 字节" << endl;
            for(i = 0; i < rest_toread_byte_num; i++) {
                content[read_byte_count] = addr[f_offset % 512 + i];
                read_byte_count++;
            }

            f_offset += rest_toread_byte_num;
            break;
        } else {
            // 需要读取本块剩余所有字节
            const int bn = mapBlk(has_read_blk_num);
            Buffer* buf = BM -> readBuf(bn);
            const char* addr = buf -> b_addr;

            // cout << "读取 " << has_read_blk_num << " 块盘块，" << blk_rest_byte_num << " 字节" << endl;
            for(i = 0; i < blk_rest_byte_num; i++) {
                content[read_byte_count] = addr[f_offset % 512 + i];
                read_byte_count++;
            }

            f_offset += blk_rest_byte_num;
            blk_rest_byte_num = 0;
        }
    }

    return toread_byte_num;
}

/**
 * 将内容写入文件
 * 以当前 f_offset 为起始字节开始写入
 * 返回写入的字节数
 */
int File::write(char* content, int length) {
    const int rest_byte_num = max(getFileSize() - f_offset, 0); // f_offset 之后文件剩余的大小
    const int towrite_byte_num = min(rest_byte_num, length);
    const int append_byte_num = (length - rest_byte_num) > 0? (length - rest_byte_num): 0; // 需要新写入的大小

    int i, j;
    int write_byte_count = 0;
    int has_write_blk_num = f_offset / 512; // 已经写过的块数（逻辑块），根据 f_offset 确定
    int blk_rest_byte_num = 512 - (f_offset - has_write_blk_num * 512); // 在当前盘块内，当前文件指针后，还剩余多少字节未读取

    while(write_byte_count < towrite_byte_num) {
        if(blk_rest_byte_num == 0) {
            blk_rest_byte_num = 512;
            has_write_blk_num++;
        }

        const int rest_towrite_byte_num = towrite_byte_num - write_byte_count; // 剩余待写入字节数
        if(blk_rest_byte_num >= rest_towrite_byte_num) { // 如果当前块剩余字节数大于剩余所需写入字节数
            // 只需要重新覆盖写本块，便可以完成本轮写入
            const int bn = mapBlk(has_write_blk_num);
            Buffer* buf = BM -> readBuf(bn);
            char* addr = buf -> b_addr;

            // cout << "写入 " << bn << " 块盘块，" << rest_towrite_byte_num << " 字节" << endl;
            for(i = 0; i < rest_towrite_byte_num; i++) {
                addr[f_offset % 512 + i] = content[write_byte_count];
                // cout << "写入字符 " << content[write_byte_count] << endl;
                write_byte_count++;
            }

            f_offset += rest_towrite_byte_num;
            BM -> dwriteBuf(buf);
            break;
        } else {
            // 需要写入整块
            const int bn = mapBlk(has_write_blk_num);
            Buffer* buf = BM -> readBuf(bn);
            char* addr = buf -> b_addr;

            // cout << "写入 " << has_write_blk_num << " 块盘块，" << blk_rest_byte_num << " 字节" << endl;
            for(i = 0; i < blk_rest_byte_num; i++) {
                addr[f_offset % 512 + i] = content[write_byte_count];
                // cout << "写入字符 " << content[write_byte_count] << endl;
                write_byte_count++;
            }

            f_offset += blk_rest_byte_num;
            BM -> dwriteBuf(buf);
            blk_rest_byte_num = 0;
        }
    }

    has_write_blk_num = f_offset / 512; // 已经写过的块数（逻辑块），根据 f_offset 确定
    blk_rest_byte_num = 512 - (f_offset - has_write_blk_num * 512); // 在当前盘块内，当前文件指针后，还剩余多少字节未读取

    const int append_pre_byte_num = (append_byte_num > blk_rest_byte_num)? blk_rest_byte_num: append_byte_num; // 在最后一块盘块需要继续写入的字节数
    const int append_all_blk_num = (append_byte_num - append_pre_byte_num) / 512; // 需要新增的盘块数（完整的盘块）
    const int append_aft_byte_num = (append_byte_num - append_pre_byte_num) - 512 * append_all_blk_num; // 需要在写完完整盘块后，仍然需要写入的字节数

    
    int pre_bn;
    Buffer* pre_buf;
    if(getFileSize() == 0) {
        // 如果是空文件
        pre_buf = applyNewBlk();
        pre_bn = mapBlk(0);
    } else {
        pre_bn = mapBlk(has_write_blk_num);
        pre_buf = BM -> readBuf(pre_bn);
    }
    char* pre_addr = pre_buf -> b_addr;
    // cout << "写入 " << pre_bn << " 块盘块，" << append_pre_byte_num << " 字节" << endl;
    for(int i = 0; i < append_pre_byte_num; i++) {
        pre_addr[f_offset % 512 + i] = content[write_byte_count]; 
        // cout << "写入字符 " << content[write_byte_count] << endl;
        write_byte_count++;
    }
    BM -> dwriteBuf(pre_buf);
    f_minode -> m_size += append_pre_byte_num;

    for(int i = 0; i < append_all_blk_num; i++) {
        // 每次分配一个盘块
        Buffer* buf = applyNewBlk();
        char* addr = buf -> b_addr;
        // cout << "写入 " << buf -> b_blk_no << " 块盘块，" << 512 << " 字节" << endl;
        for(j = 0; j < 512; j++) {
            addr[j] = content[write_byte_count]; 
            // cout << "写入字符 " << content[write_byte_count] << endl;
            write_byte_count++;
        }
        BM -> dwriteBuf(buf);
        f_minode -> m_size += 512;
    }

    Buffer* aft_buf = applyNewBlk();
    char* aft_addr = aft_buf -> b_addr;
    // cout << "写入 " << aft_buf -> b_blk_no << " 块盘块，" << append_aft_byte_num << " 字节" << endl;
    for(int i = 0; i < append_aft_byte_num; i++) {
        aft_addr[i] = content[write_byte_count]; 
        // cout << "写入字符 " << content[write_byte_count] << endl;
        write_byte_count++;
    }
    BM -> dwriteBuf(aft_buf);
    f_minode -> m_size += append_aft_byte_num;

    if(append_all_blk_num + append_aft_byte_num != 0) { // 如果申请了新的盘块
        f_minode -> m_mode |= MemINode::IUPD;
    }
    
    return write_byte_count;
}

/**
 * 删除文件中一部分内容
 * 以当前 f_offset 为起始开始删除
 * 删除的内容保存在 content 中（如果 content 不为 nullptr）
 * content 为 nullptr 则不保存删除的内容
 * 返回删除的字节数
 */
int File::remove(char* content, int length) {
    const int rest_byte_num = max(getFileSize() - f_offset, 0); // f_offset 之后文件剩余的大小
    const int tormv_byte_num = min(rest_byte_num, length);

    if(tormv_byte_num == 0) {
        return 0;
    }

    const int of_offset = f_offset; // 记录当前的文件偏移位置
    char* rest_saver = new char[rest_byte_num];
    read(rest_saver, rest_byte_num); // 将剩余部分读出
    trunc(getFileSize() - rest_byte_num); // 保留待删除部分之前的所有内容
    if(rest_byte_num > tormv_byte_num) {
        // 如果不需要将后面所有内容都删除，则还需要重新将 rest_saver 的尾部重新写入
        f_offset = of_offset;
        write(&rest_saver[tormv_byte_num], rest_byte_num - tormv_byte_num);
    }

    if(content != nullptr) {
        for(int i = 0; i < tormv_byte_num; i++) {
            content[i] = rest_saver[i];
        }
    }
    delete[] rest_saver;

    f_offset = of_offset;
    f_minode -> m_mode |= MemINode::IUPD;
    return tormv_byte_num;
}

/**
 * 将文件截断
 * 根据输入，保留文件前面若干字节
 */
void File::trunc(const int size) {
    const int osize = getFileSize(); // original size
    const int oblk_num = getBlkNum(); // original block amount

    const int nblk_num = size / 512 + (size % 512 == 0? 0: 1); // new block amount，即截断后的文件盘块数
    const int rblk_num = oblk_num - nblk_num; // remove block amount，即要删除的盘块数

    int* addr = f_minode -> m_addr;

    int i, j, k;
    for(i = 0; i < rblk_num; i++) {
        if(addr[9] != 0 || addr[8] != 0) {
            Buffer* index1_blk = BM -> readBuf(addr[9] != 0? addr[9]: addr[8]); // 一级索引块
            int* b_addr1 = (int*)(index1_blk -> b_addr);

            for(j = 127; j >= 0; j--) {
                if(b_addr1[j] != 0) { // 找到最后一个二级索引块
                    Buffer* index2_blk = BM -> readBuf(b_addr1[j]); // 二级索引块
                    int* b_addr2 = (int*)(index2_blk -> b_addr);

                    for(k = 127; k >= 0; k--) {
                        if(b_addr2[k] != 0) { // 找到最后一个盘块
                            FS -> freeBlock(b_addr2[k]);
                            b_addr2[k] = 0;
                            break;
                        }
                    }

                    if(k == 0) {
                        // 如果释放了二级索引块指向的第一块盘块，则二级索引不再需要
                        FS -> freeBlock(b_addr1[j]);
                        b_addr1[j] = 0;
                        BM -> dwriteBuf(index2_blk);
                        break;
                    }
                    BM -> freeBuf(index2_blk);
                    break;
                }
            }

            if(j == 0) {
                // 如果释放了一级索引块指向的第一个盘块，则这个一级索引不再需要
                FS -> freeBlock(addr[9] != 0? addr[9]: addr[8]);
                f_minode -> m_mode |= MemINode::IUPD;
                if(addr[9] != 0) {
                    addr[9] = 0;
                } else {
                    addr[8] = 0;
                }
                BM -> dwriteBuf(index1_blk);
                continue;
            }

            BM -> freeBuf(index1_blk);
            continue;
        }

        if(addr[7] != 0 || addr[6] != 0) {
            Buffer* index1_blk = BM -> readBuf(addr[7] != 0? addr[7]: addr[6]); // 一级索引块
            int* b_addr1 = (int*)(index1_blk -> b_addr);

            for(j = 127; j >= 0; j--) {
                if(b_addr1[j] != 0) { // 找到最后一个盘块
                    FS -> freeBlock(b_addr1[j]);
                    b_addr1[j] = 0;
                    break;
                }
            }

            if(j == 0) {
                // 如果释放了一级索引块指向的第一个盘块，则这个一级索引不再需要3
                FS -> freeBlock(addr[7] != 0? addr[7]: addr[6]);
                f_minode -> m_mode |= MemINode::IUPD;
                if(addr[7] != 0) {
                    addr[7] = 0;
                } else {
                    addr[6] = 0;
                }
                BM -> dwriteBuf(index1_blk);
                continue;
            }

            BM -> freeBuf(index1_blk);
            continue;
        }

        for(j = 5; j >= 0; j--) {
            if(addr[j] != 0) { // 找到最后一个直接映射的盘块
                FS -> freeBlock(addr[j]);
                f_minode -> m_mode |= MemINode::IUPD;
                addr[j] = 0;
                break;
            }
        }
    }

    f_minode -> m_size = size;
}