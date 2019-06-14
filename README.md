Saptapper
=========
[![Travis Build Status](https://travis-ci.org/loveemu/saptapper.svg?branch=master)](https://travis-ci.org/loveemu/saptapper) [![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/8gdychs5ftgijyui/branch/master?svg=true)](https://ci.appveyor.com/project/loveemu/saptapper/branch/master)

Automated GSF ripper tool created by Caitsith2, reimplemented by loveemu from scratch.
<https://github.com/loveemu/saptapper>

Caitsith2's original version can be found at GSF Central
<http://www.caitsith2.com/gsf/ripping.html>

Downloads
---------

- [Latest release](https://github.com/loveemu/saptapper/releases/latest)

Usage
-----

Syntax: `saptapper {OPTIONS} romfile`

### Options

|Argument        |Description                                                 |
|----------------|------------------------------------------------------------|
|`-h`, `--help`  |Show this help message and exit                             |
|`--inspect`     |Show the inspection result without saving files and quit    |
|`-d[directory]` |The output directory (the default is the working directory) |
|`-o[basename]`  |The output filename (without extension)                     |
|`romfile`       |The ROM file to be processed                                |

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
