# ShapeLibPlusPlus

This is essentially a fork of ShapeLib by Frank Warmerdam. The original ShapeLib was offered under a dual MIT-LGPL license.
This fork was prepared by Bryan Poling at Sentek Systems, LLC and is distributed under just the more permissive MIT license.

The original ShapeLib was prepared as a standalone C library, meant to be compiled and installed and then dynamically linked to.
It included various executable utilities and scripts. In contrast, this fork is prepared in a manner more appropriate for use in
proprietary software packages or free software packages intended to be distributed in binary form. The core library source files
have been excised from ShapeLib, discarding all of the utilities and scripts. C++ wrappers provide higher-level interfaces for
reading and writing (initially just for writing) georegistered shapefiles. To use this library, simply include all of the source
files in your C++ project and the library will be statically linked into your program. Include the appropriate c++ header(s)
where you need to use the library.

Compatibility
-------------

I strive to write the C++ wrappers in a way that works on all major platforms and with all major compilers, but I am not able to
test all combinations. My testing is primarily with GCC on Linux and, to a lesser extent, with Visual Studio on Windows. Since
this exposes C++ interfaces to the library, there you should have no expectations regarding ABI compatibility. You should always
include the source files from this library in your project and compile them as part of your normal build process to ensure
proper linking.
