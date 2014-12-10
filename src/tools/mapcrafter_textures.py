#!/usr/bin/env python

import argparse
import errno
import glob
import os
import subprocess
import sys
import zipfile

dirs = ("", "entity", "entity/chest", "colormap", "blocks")
assets = "assets/minecraft/textures/"
files = [
	("entity/chest/normal.png", assets + "entity/chest/normal.png"),
	("entity/chest/normal_double.png", assets + "entity/chest/normal_double.png"),
	("entity/chest/ender.png", assets + "entity/chest/ender.png"),
	("entity/chest/trapped.png", assets + "entity/chest/trapped.png"),
	("entity/chest/trapped_double.png", assets + "entity/chest/trapped_double.png"),
	("colormap/foliage.png", assets + "colormap/foliage.png"),
	("colormap/grass.png", assets + "colormap/grass.png"),
        ("endportal.png", assets + "entity/end_portal.png")
]

def has_imagemagick():
	try:
		subprocess.check_output("convert")
                return True
	except subprocess.CalledProcessError:
		return True
	except OSError as e:
		if e.errno == errno.ENOENT:
			return False
		raise e

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description="Extracts from a Minecraft Jar file the textures required for mapcrafter.")
	parser.add_argument("-f", "--force", 
					help="forces overwriting eventually already existing textures",
					action="store_true")
	parser.add_argument("jarfile",
					help="the Minecraft Jar file to use",
					metavar="<jarfile>")
	parser.add_argument("outdir",
					help="the output texture directory",
					metavar="<outdir>")
	args = vars(parser.parse_args())
	
	jar = zipfile.ZipFile(args["jarfile"])
	
	for dir in dirs:
		if not os.path.exists(os.path.join(args["outdir"], dir)):
			os.mkdir(os.path.join(args["outdir"], dir))
	
	print("Extracting block images:")
	found, extracted, skipped = 0, 0, 0
	for info in jar.infolist():
		if info.filename.startswith("assets/minecraft/textures/blocks/") and info.filename != "assets/minecraft/textures/blocks/":
			filename = info.filename.replace("assets/minecraft/textures/", "")
			filename = os.path.join(args["outdir"], filename)
			found += 1
			
			if os.path.exists(filename) and not args["force"]:
				skipped += 1
				continue
			
			fin = jar.open(info)
			fout = open(filename, "wb")
			fout.write(fin.read())
			fin.close()
			fout.close()
			extracted += 1
	
	print(" - Found %d block images." % found)
	print(" - Extracted %d." % extracted)
	print(" - Skipped %d (Use -f to force overwrite)." % skipped)
	
	print("")
	print("Extracting other textures:")
	
	for filename, zipname in files:
		try:
			info = jar.getinfo(zipname)
			filename = os.path.join(args["outdir"], filename)
			if os.path.exists(filename) and not args["force"]:
				print(" - Extracting %s ... skipped." % filename)
			else:
				fin = jar.open(info)
				fout = open(filename, "wb")
				fout.write(fin.read())
				fin.close()
				fout.close()
				print(" - Extracting %s ... extracted." % filename)
		except KeyError:
			print(" - Extracting %s ... not found!" % filename)
	
	if not has_imagemagick():
		print("")
		print("Warning: imagemagick is not installed (command 'convert' not found).")
		print("Install imagemagick to enable automatic texture fixes (to prevent libpng warnings).")
	else:
		for filename in glob.glob(os.path.join(args["outdir"], "blocks", "hardened_clay*.png")):
			if os.path.exists(filename):
				subprocess.check_call(["convert", filename, filename])
		
		filename = os.path.join(args["outdir"], "blocks", "red_sand.png")
		if os.path.exists(filename):
			subprocess.check_call(["convert", filename, filename])
		
		filename = os.path.join(args["outdir"], "blocks", "glass_pane_top_white.png")
		if os.path.exists(filename):
			subprocess.check_call(["convert", filename, "-type", "TrueColorMatte", "-define", "png:color-type=6", filename])
