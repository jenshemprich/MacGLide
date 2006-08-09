Put the src for the following archives in the "Libraries"-directory:

DevIL >= 1.6.7
jpeg-6b
lpng > 1.2.8
tiff -> 3.7.4
zlib >= 1.2.3
libzip >= 0.61

Unzip them all and remvove the version number from the dir name. Then move the files into the each correspondig "*-src" directory (created by Subversion).

If you upgrade a component, you should remove the old content of the corresponding *_src directory.

The Library directory also contains header files for some projects. These are copies of the original source file but changed to allow compilation in CW for OS9 without changing the original sources (or convince the authors to add support for CW).

These headers are then injected into the project via precompiled headers.
