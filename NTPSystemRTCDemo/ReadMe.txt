Doxygen Format
/** @pagegroup{pageExamples,NTPSystemRTCDemo,NTP & Real-Time Clock}

NTP-SYSTEM-RTC DEMO APPLICATION

HARDWARE SETUP: Tested with MOD54415 on a MOD-DEV-70 hardware rev 1.93.

This example demonstrates the following features:

- Updates the system time with the current UTC time pulled from an available
  NTP server (pool.ntp.org). If your module is set to static IP address
  settings, then you will need to manually assign a DNS server address for
  this feature to work.

- Take the current system time and save it to the real-time clock (RTC). The
  RTC will then be running in sync with the system time.

- Take the current time in the RTC and set the system time information. The
  system time will then be running in sync with the RTC.

- When the device or module is reset or powered on again after a recent power
  loss, the system time will be reset, but the RTC can continue running until
  the the super-capacitor is discharged. The software-reset option is
  available to demonstrate the RTC's ability to retain saved time information
  once it is properly set.

- Set the local time zone information with a call to tzsetchar(). For more
  more information on this function, refer to "Chapter 14 - NBTime Library"
  of the NetBurner Runtime Libraries PDF document.  In this example, Pacific
  time zone information is used as a demonstration.

- Display the current system and RTC times, and local time zone information
  if applicable. It is preferred that the RTC store the the UTC time, not
  local time.

- In order to take advantage of setting and getting time information from
  the real-time clock (RTC), a device that contains an RTC component or a
  module that is mounted on a development board with an RTC component must be
  used. Current RTC components supported in this example at the time of this
  writing are the Intersil X1226 and NXP PCF8563.

*/