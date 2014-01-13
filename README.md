Saptapper
=========

My rewrite of Caitsith2's automated GSF ripper tool.

Usage
-----

Syntax: saptapper <GBA Files>

### Options ###

--help
  : Show this help

-v, --verbose
  : Output ripping info to STDOUT

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
