Doxygen Format
/** @pagegroup{pageExamples,Profile,Profiler}

 This is a very simple profiler program. The way the NBRTOS_TIME functions operate
 is that there is an additional routine that runs during task switching to record
 the tick and tickfraction since the task swapped in (runtime since last change).
 
 This feature has overhead and you normally only use it for development.
 To enable profiling:
 
 1. Edit `\nburn\nbrtos\include\prefef.h` and uncomment: @code #define NBRTOS_TIME(1) @endcode
 2. Rebuild the system libraries
 
 The following functions are now available:
 
 uint32_t GetCurrentTaskTime( uint32_tconst TotalTicks );
 Returns the number of time ticks the current task has run. TotalTicks = the
 total number of ticks recorded.
 
 void ShowTaskTimes( void );
 Print a list of tasks, times and percentages to stdout (normally uart0)
 
 void ClearTaskTimes( void );
 Resets all task times to 0

*/