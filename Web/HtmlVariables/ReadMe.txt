Doxygen Format
/** @pagegroup{Web,HtmlVariables,HTMLVariable}

This example illustrates how to use dynamic HTML content with variables and
 function call parameters embedded in HTML code. The HTML tags VARIABLE and
 FUNCTIONCALL enable you to display application variables and call
 application functions directly from the HTML code.
 
 In this example, the device's IP, Mask, Gateway, and DNS Server are displayed
 dynamically on the web page. The page also displays an uptime counter which
 updates when the page is refreshed.
 
 Note for NBEclipse Users: This example requires that the auto-generated file
 htmldata.cpp have a path to include the htmlvar.h header file. To add the
 path in NBEclipse:
 
Right-click on your project and select properties
Select "C/C++ Build" options
Select "GNU C++ Compiler" -> Directories
Use the "+" in the include path box to add the project's "HtmlVariables\html"
 folder to the list of include paths.

*/