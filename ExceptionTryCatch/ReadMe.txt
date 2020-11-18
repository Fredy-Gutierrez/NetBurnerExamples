Doxygen Format
/** @pagegroup{pageExamples,pageExamples-ExceptionTryCatch,ExceptionTryCatch}

 This example program illustrates how to use C++ exceptions. It
 will throw an exception and verify it can be caught. The output
 is displayed to stdout, which can be viewed with MTTTY on the
 debug serial port.
 
 COMPILATION INSTRUCTIONS
 To keep the application size small for users who do not need
 C++ exceptions, this feature is disabled by default. To enable
 exceptions you need to add the "-fexceptions" and flag to the 
 C++ build options. In addition, "-frtti" also needs to be added.
 If these options are not enabled, the compiler output console 
 window will notify you.
 
 To enable Exceptions in NBEclipse:
 - Right-click on your project and select Properties
 - Select "GNU C++ Compiler"
 - Select "Miscellaneous"
 - In the Other Flags field, add "-fexceptions -frtti"

*/