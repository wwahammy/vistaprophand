The XMP toolkit needs an external XML parser. The source from Adobe is written to use Expat,
although adapters for other parsers can easily be written. The most recent version of Expat used
with XMP is 2.0.0. To use Expat:

1. Obtain a copy of the Expat distribution. One good place is SourceForge:
	http://sourceforge.net/projects/expat/

2. Place Expat's lib directory within .../third-party/expat. I.e. as a sibling of this file.

For Expat version 2.0.0 the contents of .../third-party/expat/lib are:

	ascii.h
	asciitab.h
	expat.h
	expat_external.h
	iasciitab.h
	internal.h
	latin1tab.h
	macconfig.h
	nametab.h
	utf8tab.h
	winconfig.h
	xmlparse.c
	xmlrole.c
	xmlrole.h
	xmltok.c
	xmltok.h
	xmltok_impl.c
	xmltok_impl.h
	xmltok_ns.c
