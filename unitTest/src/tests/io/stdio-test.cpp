#include "CppUTest/TestHarness.h"
#include <stdio.h>
#include <iosys.h>
#include <serial.h>
#include <limits.h>
#include <string.h>

TEST_GROUP(stdio_test)
{
	void setup()
	{

	}

	void teardown()
	{

	}
};

TEST(stdio_test,formatted_iprintf_output)
{
	char output[128];
	memset(output, 0xFF, sizeof(output));
	BYTES_EQUAL(0xFF,output[96]);

	// Full ascii output
	CHECK_EQUAL(95,siprintf(output," !\"#$%%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"));
	STRCMP_EQUAL(" !\"#$\%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",output);
	BYTES_EQUAL(0x0,output[95]); // Check for null termination
	BYTES_EQUAL(0xFF,output[96]); // Don't write past null termination

	// min/max numbers
	siprintf(output,"%d",INT_MAX);
	STRCMP_EQUAL("2147483647",output);
	siprintf(output,"%d",INT_MIN);
	STRCMP_EQUAL("-2147483648",output);
	siprintf(output,"%u %d",0,0);
	STRCMP_EQUAL("0 0",output);
	siprintf(output,"%u",UINT_MAX);
	STRCMP_EQUAL("4294967295",output);

	// formatting options
	const char* s = "Hello";
	const char* result = "\t.     Hello.\n\t.Hello     .\n\t.     Hello.\n";
    siprintf(output,"\t.%10s.\n\t.%-10s.\n\t.%*s.\n", s, s, 10, s);
    STRCMP_EQUAL(result,output);
    siprintf(output,"%c %%", 65);
    STRCMP_EQUAL("A %",output);
    siprintf(output,"%i %d %.6i %i %.0i %+i %u", 1, 2, 3, 0, 0, 4, -1);
    STRCMP_EQUAL("1 2 000003 0  +4 4294967295",output);
    siprintf(output,"%x %x %X %#x", 5, 10, 10, 6);
    STRCMP_EQUAL("5 a A 0x6",output);
    siprintf(output,"%o %#o %#o", 10, 10, 4);
    STRCMP_EQUAL("12 012 04",output);
}

TEST(stdio_test,formatted_printf_output)
{
	char output[128];
	memset(output, 0xFF, sizeof(output));
	BYTES_EQUAL(0xFF,output[127]);

    // floats
    sprintf(output,"%f %.0f %.32f", 1.5, 1.5, 1.3);		// rounding
    STRCMP_EQUAL("1.500000 2 1.30000000000000004440892098500626",output);
    sprintf(output,"%.2f %5.2f", 1.5, 1.5);				// padding
    STRCMP_EQUAL("1.50  1.50",output);
    sprintf(output,"%E %e", 1.5, 1.5);					// scientific
    STRCMP_EQUAL("1.500000E+00 1.500000e+00",output);
}

TEST(stdio_test,scanf_formats)
{
    int i, j;
    float x, y;
    char str1[10], str2[4];
    wchar_t warr[2];
    char input[] = u8"25 54.32E-1 Thompson 56789 0123 56";
    // parse as follows:
    // %d: an integer 
    // %f: a floating-point value
    // %9s: a string of at most 9 non-whitespace characters
    // %2d: two-digit integer (digits 5 and 6)
    // %f: a floating-point value (digits 7, 8, 9)
    // %*d an integer which isn't stored anywhere
    // ' ': all consecutive whitespace
    // %3[0-9]: a string of at most 3 digits (digits 5 and 6)
    // %2lc: two wide characters, using multibyte to wide conversion
    int ret = sscanf(input, "%d%f%9s%2d%f%*d %3[0-9]",
                     &i, &x, str1, &j, &y, str2, warr);
    CHECK_EQUAL(25,i);
    DOUBLES_EQUAL(54.32E-1,x,0.0001);
    STRCMP_EQUAL("Thompson",str1);
    CHECK_EQUAL(56,j);
    DOUBLES_EQUAL(789,y,0.0001);
    STRCMP_EQUAL("56",str2);
}
