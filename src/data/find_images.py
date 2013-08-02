#!/usr/bin/env python

import sys
import os
import zipfile

dirs = ("chest", "colormap")
files = {
	"chest/normal.png" : "assets/minecraft/textures/entity/chest/normal.png",
	"chest/ender.png" : "assets/minecraft/textures/entity/chest/ender.png",
	"chest/normal_double.png" : "assets/minecraft/textures/entity/chest/normal_double.png",
	"colormap/foliage.png" : "assets/minecraft/textures/colormap/foliage.png",
	"colormap/grass.png" : "assets/minecraft/textures/colormap/grass.png",
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
		
	print "Extracting block images:"
	found, extracted, skipped = 0, 0, 0
	for info in jar.infolist():
		if info.filename.startswith("assets/minecraft/textures/blocks/"):
			filename = info.filename.replace("assets/minecraft/textures/", "")
			found += 1
			
			if os.path.exists(filename) and not force:
				skipped += 1
				continue
			
			fin = jar.open(info)
			fout = open(filename, "w")
			fout.write(fin.read())
			fin.close()
			fout.close()
			extracted += 1
	
	print " - Found %d block images." % found
	print " - Extracted %d." % extracted
	print " - Skipped %d (Use -f to force overwrite)." % skipped
	
	print ""
	print "Extracting other textures:"
	
	for dir in dirs:
		if not os.path.exists(dir):
			os.mkdir(dir)
	
	for filename, zipname in files.items():
		try:
			print " - Extracting" , filename , "...",
			info = jar.getinfo(zipname)
			if os.path.exists(filename) and not force:
				print "skipped."
			else:
				fin = jar.open(info)
				fout = open(filename, "w")
				fout.write(fin.read())
				fin.close()
				fout.close()
				print "extracted."
		except KeyError:
			print "not found!"
		
