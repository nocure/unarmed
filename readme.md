# I am unARMed

This is an embedded system for me to evaluate new stuff, It used to run on a tq2440 board (An S3C2440 demo board), can be run on Lusimus ([nocure / lusimus](https://github.com/nocure/lusimus)) too.

Originally the OS was ThreadX, it's licensed to a company that I worked with, can't release it to public, so I ported entire system to UCOS II. There were FileX and UsbX, removed for same reason.

I intend to replace FileX with FatFS as file system, FatFS is here, can be successfully compiled to a lib, but I havn't got time to link FatFS into system yet. All file manipulating commands are removed at the moment.

Each directory contains an Eclipse project, You can grab whole package from GitHub, place it in a directory (i.e. c:\unarmed), then create a workspace in the root directory (c:\unarmed), import projects into workspace.  
Project "main" is ELF execuable, compile main last, rest are libs.

I hope there are stuff worthwhile.

Have fun...