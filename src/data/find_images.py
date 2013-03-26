#!/usr/bin/env python

import sys
import os
import zipfile

files = {
	"chest.png" : "item/chest.png",
	"enderchest.png" : "item/enderchest.png",
	"largechest.png" : "item/largechest.png",
}

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "Usage: ./find_images.py [-f] <minecraft.jar>"
		sys.exit(1)
		
	force = False
	path = sys.argv[1]
	if len(sys.argv) >= 3:
		force = sys.argv[1] == "-f"
		path = sys.argv[2]
		
	jar = zipfile.ZipFile(path)
	
	for filename, path in files.items():
		try:
			info = jar.getinfo(path)
			print filename, "found...",
			if os.path.exists(filename) and not force:
				print "skipping because it already exists (use -f to force)"
			else:
				fin = jar.open(info)
				fout = open(filename, "w")
				fout.write(fin.read())
				fin.close()
				fout.close()
				print "ok"
		except KeyError:
			print filename, "not found!"
			continue
