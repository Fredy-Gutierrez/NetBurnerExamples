Doxygen Format
/** @pagegroup{pageExamples-RTOS,pageExamples-RTOS-OSFlags,OSFlags Example Program}

This illustrates how OSFlags can be used to pend on multiple events.
`UserMain()` creates 3 tasks, each of which will set a OSFlag after a
time delay of some number of seconds. UserMain will then pend and
block until ANY of the 3 flags are set. You can also modify the example
to pend until ALL of the 3 flags are set.

*/