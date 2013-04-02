#!/usr/bin/env python

import sys
import os
import zipfile

files = {
	"chest.png" : "item/chest.png",
	"enderchest.png" : "item/enderchest.png",
	"largechest.png" : "item/largechest.png",
	"foliagecolor.png" : "misc/foliagecolor.png",
	"grasscolor.png" : "misc/grasscolor.png",
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
	
	if not os.path.exists("blocks"):
		os.mkdir("blocks")
	
	jar = zipfile.ZipFile(path)
	for info in jar.infolist():
		filename = info.filename
		if info.filename.startswith("textures/blocks/"):
			filename = info.filename.replace("textures/", "")
		else:
			extract = False
			for name, path in files.items():
				if filename == name:
					extract = True
			if not extract:
				continue
				
		print "found", filename, "...",
		if os.path.exists(filename) and not force:
			print "skipping"
		else:
			fin = jar.open(info)
			fout = open(filename, "w")
			fout.write(fin.read())
			fin.close()
			fout.close()
			print "ok"
			
	print "use -f to overwrite already existing files"
