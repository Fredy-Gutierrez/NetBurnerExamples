# The digest value is normally displayed in NBEclipse by adding the -D to the compcode project properties.
#  
# Alternatively, you can use this command line example of compcode to calculate and display the SHA1 digest of the application.
# Modify the -P option to match your platform, and the -R memory values to match your flash memory range.  
# 
compcode SHA1Digest.s19 SHA1Digest_APP.s19 -R 0xffc08000 0xFFC80000 -PMOD5270 -D
