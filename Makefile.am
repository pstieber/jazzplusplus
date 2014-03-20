ACLOCAL_AMFLAGS=-I m4

SUBDIRS = src conf

# Here are some extra things to distribute.
# Things to fix:
#   The docs are missing because they are so large.
#   Subversion directories are included.
EXTRA_DIST = bootstrap song scripts midinetd mswin contrib bitmaps rebuild

really-clean: maintainer-clean
	cd bitmaps; make clean
	rm -f configure Makefile.in src/Makefile.in conf/Makefile.in aclocal.m4 src/config.h.in

htmldocs:
	doxygen jazz.doxy

# New stuff
Jazz++.app: Info.plist src/jazz
	-mkdir Jazz++.app
	-mkdir Jazz++.app/Contents
	-mkdir Jazz++.app/Contents/MacOS
	-mkdir Jazz++.app/Contents/Resources
	-mkdir Jazz++.app/Contents/Resources/English.lproj
	cp Info.plist Jazz++.app/Contents/
	echo -n "Jazz++" > Jazz++.app/Contents/PkgInfo
	cp src/jazz Jazz++.app/Contents/MacOS/Jazz++

libs:
	dylibbundler -od -b -x ./Jazz++.app/Contents/MacOS/Jazz++ -d ./Jazz++.app/Contents/libs/
