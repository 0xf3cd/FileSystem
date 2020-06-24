# FileSystem

* TODO（如果之后还能想到有这些 bugs && 我有时间的话
    * 进入文件系统 `format` 后会出问题，但是 `format` 后先 `mkdir` 就能恢复正常
        * 可能是一些全局变量没有刷新
    * 有内存泄露的问题，需要慢慢找
    * 有把函数内局部变量的地址赋值给全局变量的情况，这有可能导致问题 
        * 如 `SuperBlockManager::loadSuperBlock` 中
    