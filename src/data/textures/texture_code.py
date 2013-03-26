#!/usr/bin/env python

# developer stuff:
# lists the block textures in the 'blocks' dir and generates the c++ texture code

import os
import re

def lowercase(name):
	return re.sub("[A-Z]", lambda m: "_" + m.group(0).lower(), name)

files = []
for filename in os.listdir("blocks"):
	if not filename.endswith(".png"):
		continue
	name = filename.replace(".png", "")
	const_name = lowercase(name).upper()
	files.append((name, const_name))

files.sort()

print "\tTextureImage"
for name, const_name in files:
	last = ","
	if name == files[-1][0]:
		last = ";"
	print "\t\t%s = TextureImage(\"%s\")%s" % (const_name, name, last)

print ""
print "\tstatic const int texture_count = %d;" % len(files)
print "\tTextureImage *textures[texture_count] = {"
print "\t\t&%s" % ", &".join(map(lambda f: f[1], files))
print "\t};"
