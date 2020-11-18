Doxygen Format
/** @pagegroup{WebConfig, WebConfigCustom, NetBurner Custom Web Config Demo}

This program shows the basics of using NetBurner's config system. The config system is used both to
 store boot and interface information, such as default BaudRate and current IP settings, as well as
 provide for the user to define and store their own persistent variables.
 
 This example is similare to the BasicWebConfig example, but implements the content with JavaScript and jQuery. It also
 demonstrates how to store and display your own custom application data in the AppData section of the configuration system. 
 
 In this example, we've created some objects that are created when the device first boots, and whose
 values persist after the device has been power cycled. In addition, we've added a custom, styled web
 interface that users can use to view and set the values.The currently available config objects that
 can be used are as follows:

   - `config_obj`           // base object, used as a container for other objects
   - `config_bool`          // bool
   - `config_int`           // int
   - `config_double`        // double
   - `config_string`        // string
   - `config_chooser`       // multiple choice option
   - `config_pass`          // password
   - `config_IPADDR4`       // IP4 Address
   - `config_IPADDR6`       // IP6 Address
   - `config_MACADR`        // MAC Address
 
 Note that because we are only processing the config object, we are able to avoid writing a custom POST
 handler that is used in the other examples .If you include more to your web interface than is provided
 here, that will need to be incorporated as well.

*/