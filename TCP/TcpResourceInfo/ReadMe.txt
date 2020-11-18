Doxygen Format
/** @pagegroup{pageExamples-TCP,pageExamples-TCP-TcpResourceInfo,Resource Info Example}

 This example demonstrates the use of functions to obtain the number of free
 system buffers and the number of free network sockets. A socket will be consumed
 when you listen or connect. For example, the web server will consume one socket
 listening to port 80 for incoming connections.

 If the system is out of buffers or sockets, it will not be able to accept any
 incoming connections or make any outgoing connections.

 `int32_t GetFreeCount()`         returns the number of free system buffers
 `int32_t GetFreeScoketCount()`   returns the number of free sockets/file descriptors, not included extra fds
 `int32_t GetFreeExtraFDCount()`  returns the number of extra file descriptors

*/