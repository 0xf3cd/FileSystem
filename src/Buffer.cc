#include "Buffer.h"

Buffer::Buffer() {
    b_flags = 0;
    b_forw = nullptr;
    b_back = nullptr;
    b_wcount = 0;
    b_addr = nullptr;
    b_blk_no = 0;
}

Buffer::~Buffer() {
    
}