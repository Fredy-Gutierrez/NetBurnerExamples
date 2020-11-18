Doxygen Format
/** @pagegroup{pageExamples-RTOS, pageExamples-RTOS-OSMultiTaskUserInput, Multiple Task User Input}

This program illustrates how to create a task and use a mailbox to accept
user input from a serial port. This example also illustrates how to
terminate a task
 
1. UserMain task will wait pend on a mailbox for user input
2. UserInput task will block waiting for a user to type in data from the
serial port

*/