Doxygen Format
/** @pagegroup{WebConfig, WebConfigBasic, NetBurner Basic Web Config Demo}

This program shows the basics of using NetBurner's config system and presenting the information through
a dynamically generated web interface. The config system is used both to store boot and interface
information, such as default BaudRate and current IP settings, as well as provide a place for the user to define
and store their own persistent variables.
 
The code used to generate and process the web form is in `html/index.html`. This example does not use any external
JavaScript libraries, and all of the form generation code is located at the bottom of index.html. For a more
advanced example that utilized Bootstrap and jQuery, please see the CustomWebConfig example.
 
Currently available config objects that can be used are as follows:

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
handler that is used in the other examples. If you include more to your web interface than is provided
here, that will need to be incorporated as well.
*/