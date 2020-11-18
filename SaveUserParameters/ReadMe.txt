Doxygen Format
/** @pagegroup{pageExamples,SaveUserParameters,save user param}

Example to demonstrate how to read and write data to the on-board flash memory
User Parameter Area. This example is here for reverse compatibility with
tools/applications prior to NetBurner 3.0. If compatibility is not an issue,
we recommend using the Configuration App section. It is significantly more
flexible and provides much easier access. See the configuration users guide
and the configuration examples for more information.
 
The amount of flash space is equal to one flash sector, which will vary from 8k
bites to 64k bytes depending on your platform. 
  
Warnings:
 
 1. Power is interrupted during the flash programming process. We strongly recommend
    that UserParameters are not written immediately upon power-up, since it is not
    uncommon for a user to cycle a power switch on/off quickly every once in a while.

 2. Your application writes beyond the UserParameter size allocated for your
    platform (8k or 64k).
   

 Important notes on packed vs. integer alignment when using structures and classes.
 This example uses a structure named NV_SettingsStruct, which consists of types
 that are 8, 16 and 32 bits long. If you do not add an attribute tag at the
 end of the structure, the default will be integer-aligned. This means that
 padding will be added to ensure each member will be on an integer boundary.
 While this can increase execution speed, it also means that the stored data will
 be a bit larger, and doing something like an overlay or indexing into the
 structure with a pointer will not work correctly. To tell the compiler not
 to use any padding, add "__attribute__((packed));" to the end of the structure
 definition as demonstrated in this example. But again, be aware that when
 using a packed structure elsewher in your application you access is as
 packed as well.

*/