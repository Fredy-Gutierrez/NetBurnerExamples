This gives and example of how to give a board permission/lock to run specific code based on MAC address.
The basic process is we take a secret message and run it through the MD5 hash function.
We pad this secret message to some multiple of 64 bytes and save off the MD5 context....

Then on the board we want to sign we combine this partial 
MD5 context with the board MAC address and generate a 16 byte digest specific to both the
board and the secret message.

We then store this 16 byte digest somewhere (In the example we store this in the UserParameters space)

Then when we want to see if the program is runign on a board with permission we recompute this digest and 
check agains the stored value....






Preliminary steps...

Go to the Keyblob project.

Edit the text

const char * YourSecretSigningText ="This should be your company  secret message";


compile and run this on a netburner board

capture the last message..

/*Your keblob should be :*/
 const MD5_CTX YourCompanySecret =
{{2106921824u,3945495657u,2391356351u,2780313164u},
{512u,0u},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
 



Save this someplace safe...this is your use forrever compny secret.


Then copy this text to into the signboard  project....

Compile and run this on the target boardto be permitted/locked

This computes and stores a signature in UserParm space.






Now copy the company secret into the checkboard project main.cpp

Run this and it will pass..... or fail depending on if the board has been failed...





Things to modify.... or enhance...


The sign function and check function do not need to be in seperate programs...
they could be in the same program with the sign function hidden by some secret command....


In most real or significat apps your code will want to use the userpadm space for storing configuration etc...
So you need to modify the sign and check functions to store and retrieve the 16 byte digest from your storge structures...











