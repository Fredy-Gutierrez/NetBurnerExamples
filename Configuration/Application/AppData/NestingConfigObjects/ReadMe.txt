Doxygen Format
/** @pagegroup{AppData, AppDataNestingConfigObjects, Nesting Config Objects Demo}

This program shows the basics of using NetBurner's config system, specifically, how config objects
can be structured to be nested inside eachother. The config system is used both to
store boot and interface information, such as default BaudRate and current IP settings, as well as
provide for the user to define and store their own persistent variables.

In this example, we have expanded on the ClassConfig example, and created an additional config class
that is used to nest several instances of our previously defined class. This nesting allows for control
over the structure and hierarchy of the data when it is saved in the flash as a JSON blob. The currently
available config objects that can be used are as follows:

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

*/