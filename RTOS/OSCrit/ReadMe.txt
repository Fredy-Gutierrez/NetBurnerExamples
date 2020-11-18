Doxygen Format
/** @pagegroup{pageExamples-RTOS,pageExamples-RTOS-OSCrit,OSCrit Object Example}

This example will show how to create critical sections with the standard C
type function calls OSCritEnter() and OSCritLeave(), and compare them to the
C++ object type which eliminates the problem of matching each OSCritEnter()
to a corresponding OSCritLeave().
 
While this example is trivial with only a simple global variable, critical
sections need to be used for objects, such as linked lists, that could be
changed by multiple tasks. In such cases, each task would use a critical
section to ensure the modifications could be completed before a task switch
occurred.

*/