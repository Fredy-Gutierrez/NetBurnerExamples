Doxygen Format
/** @pagegroup{Platform, MOD5441X, WavPlayer, WAV File Audio Player}

This example demonstrates how to play audio files (uncompressed WAV) using the
onboard Digital to Analog Converters (DAC) on the MCF5441X processor. It is
fully capable of playing 8 bit and down converted 16 and 32 bit stereo audio at 
44100 samples per second. It has minimal support for 24 bit samples, treating it as
8 bit data with sample padding.

Before attempting to build the application, you will first need to extract the
"wav\_data\_srcs.zip" archive, containing the sample audio files converted to
data arrays. The originals used to create these source files are found in the
archive 'SD\_card.zip'. This contents directory are also intended to be placed directly onto
an (micro)SD card and files be played from it.

IMPORTANT: Since these files are external references in main.cpp, they must be aligned on an
even boundary. One way to do this is the aligned attribute. For example:
   uint8_t sinewav_440[] __attribute__(( aligned( 16 ))) = { ..... }
You can see the full declaration in the nbwav_44k .cpp file

*/
