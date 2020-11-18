Doxygen Format
/** @pagegroup{pageExamples,pageExamples-malloc,malloc}

This example demonstrates the usage of malloc and free. It
 identifies way's to track your heap space used by calling
 spaceleft() and mallinfo().Use these function to help
 ensure that applications do not run out of heap space.
 
 It is important to use both spaceleft() and mallinfo() to
 calculate the size of your heap. As this application
 demonstrates, spaceleft() alone will not always give the
 total space available to malloc.
 
 In this example, the application will allocate 3 chunks
 of space. The first is 1MB, then a 3MB, then a 512KB
 chunk. It will then free the data in a different order.
 
 After every malloc and free, a heapinfo print will occur.
 This shows the current heap space used, heap space free, and
 space reported by spaceleft().

*/