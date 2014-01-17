Saptapper
=========

Automated GSF ripper tool created by Caitsith2, revised by loveemu.
<https://github.com/loveemu/saptapper>

This modified version provides some extra options that might be useful.

Downloads
---------

- [saptapper-20140117.7z](https://drive.google.com/file/d/0B6s5ZRAO2QlARlFsMlkyOXIzMlU/edit?usp=sharing) (all ripping routines are identical to unaltered version)

Usage
-----

Syntax: saptapper <GBA Files>

### Options ###

--help
  : Show help

-v, --verbose
  : Output ripping info to STDOUT

-r
  : Output uncompressed GBA ROM

-n [count]
  : Set minigsf count

--gsf-driver-file [driver.bin] [0xXXXX]
  : Specify relocatable GSF driver block and minigsf offset

--offset-gsf-driver [0xXXXXXXXX]
  : Specify the offset of GSF driver block

--offset-selectsong [0xXXXXXXXX]
  : Specify the offset of sappy_selectsong function

--offset-songtable [0xXXXXXXXX]
  : Specify the offset of song table (well known Sappy offset)

--offset-main [0xXXXXXXXX]
  : Specify the offset of sappy_main function

--offset-init [0xXXXXXXXX]
  : Specify the offset of sappy_init function

--offset-vsync [0xXXXXXXXX]
  : Specify the offset of sappy_vsync function

--tag-gsfby [name]
  : Specify the nickname of GSF ripper

--find-freespace [ROM.gba] [size]
  : Find free space and quit

Note
----

Most of the games using the sappy driver are ripped completely and automatically with
this program. The only manual step left, is sorting and optimizing the set.

If saptapper was unable to rip the game properly, but did find one of the sappy driver
functions identifying it as being sappy compatible, an error will be written to STDERR,
identifying what function it was not able to find.

Before submitting a gsf set that has been ripped, with or without saptapper, please at the 
minimum optimize the set first.  Ideally, all sfxs/voices should be removed, all 
music/jingles kept.
