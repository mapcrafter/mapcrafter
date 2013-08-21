#!/usr/bin/env python

# developer stuff:
# lists the block textures in the 'blocks' dir and generates the c++ texture code

import sys
import os
import re

def lowercase(name):
	return re.sub("[A-Z]", lambda m: "_" + m.group(0).lower(), name)

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "You have to specify a directory with block textures!"
		sys.exit(1)
	
	files = []
	for filename in os.listdir(sys.argv[1]):
		if not filename.endswith(".png"):
			continue
		name = filename.replace(".png", "")
		const_name = lowercase(name).upper()
		files.append((name, const_name))
	
	files.sort()
	
	# code for textures.h
	
	print "\tTextureImage %s;" % ", ".join(map(lambda f: f[1], files))
	print "\tstd::vector<TextureImage*> textures;"
	
	print ""
	
	# code for textures.cpp
	
	print ""
	print ": " + ", ".join(map(lambda f: "%s(\"%s\")" % (f[1], f[0]), files)) \
			 + ", textures({" + ", ".join(map(lambda f: "&%s" % f[1], files)) + "}) {"