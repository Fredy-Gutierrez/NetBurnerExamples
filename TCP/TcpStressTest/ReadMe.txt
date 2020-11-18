Doxygen Format
/** @pagegroup{pageExamples-TCP,pageExamples-TCP-TcpStressTest,Stress Test Example}

Example to test various response times. Work in conjunction with a windows program to
exercise the test cases:
- Listen for an incomeing connection and exchange data
- Listen for an incomeing request, then make an outgoing request and exchange data
- Worse case time for sending and receiving one byte at a time both directions
- Response time of the select() function to block on multiple fds
- Bulk data transfer time

Time for the various tests will be in multiples of a time tick, which is by default
20 ticks per second (TICKS_PER_SECOND).

Be sure not to include the pc folder in your NetBurner project. It is a windows program. 

*/