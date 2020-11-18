Doxygen Format
/** @pagegroup{AppData, AppDataBasicConfigVariable, NetBurner Basic Config Variable Demo}

The config system is used both to store boot and interface information, such as default BaudRate and
current IP settings, as well as provide for the user to define and store their own persistent variables.

In this example, we've created some very basic config objects to represent the basic data types that are
created when the device first boots, and whose values persist after the device has been power cycled.
The currently available config objects that can be used are as follows:

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