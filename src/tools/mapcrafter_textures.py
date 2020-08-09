#!/usr/bin/env python

import argparse
import errno
import glob
import os
import subprocess
import sys
import zipfile

#dirs = ("", "entity", "entity/chest", "colormap", "blocks", "entity/shulker", "entity/bed")
dirs = ("", "colormap", "blocks")
assets = "assets/minecraft/textures/"
files = [
	#("entity/chest/normal.png", assets + "entity/chest/normal.png"),
	#("entity/chest/normal_double.png", assets + "entity/chest/normal_double.png"),
	#("entity/chest/ender.png", assets + "entity/chest/ender.png"),
	#("entity/chest/trapped.png", assets + "entity/chest/trapped.png"),
	#("entity/chest/trapped_double.png", assets + "entity/chest/trapped_double.png"),
	("colormap/foliage.png", assets + "colormap/foliage.png"),
	("colormap/grass.png", assets + "colormap/grass.png"),
	#("entity/shulker/shulker_black.png", assets + "entity/shulker/shulker_black.png"),
	#("entity/shulker/shulker_blue.png", assets + "entity/shulker/shulker_blue.png"),
	#("entity/shulker/shulker_brown.png", assets + "entity/shulker/shulker_brown.png"),
	#("entity/shulker/shulker_cyan.png", assets + "entity/shulker/shulker_cyan.png"),
	#("entity/shulker/shulker_gray.png", assets + "entity/shulker/shulker_gray.png"),
	#("entity/shulker/shulker_green.png", assets + "entity/shulker/shulker_green.png"),
	#("entity/shulker/shulker_light_blue.png", assets + "entity/shulker/shulker_light_blue.png"),
	#("entity/shulker/shulker_lime.png", assets + "entity/shulker/shulker_lime.png"),
	#("entity/shulker/shulker_magenta.png", assets + "entity/shulker/shulker_magenta.png"),
	#("entity/shulker/shulker_orange.png", assets + "entity/shulker/shulker_orange.png"),
	#("entity/shulker/shulker_pink.png", assets + "entity/shulker/shulker_pink.png"),
	#("entity/shulker/shulker_purple.png", assets + "entity/shulker/shulker_purple.png"),
	#("entity/shulker/shulker_red.png", assets + "entity/shulker/shulker_red.png"),
	#("entity/shulker/shulker_silver.png", assets + "entity/shulker/shulker_silver.png"),
	#("entity/shulker/shulker_white.png", assets + "entity/shulker/shulker_white.png"),
	#("entity/shulker/shulker_yellow.png", assets + "entity/shulker/shulker_yellow.png"),
	#("entity/bed/black.png", assets + "entity/bed/black.png"),
	#("entity/bed/blue.png", assets + "entity/bed/blue.png"),
	#("entity/bed/brown.png", assets + "entity/bed/brown.png"),
	#("entity/bed/cyan.png", assets + "entity/bed/cyan.png"),
	#("entity/bed/gray.png", assets + "entity/bed/gray.png"),
	#("entity/bed/green.png", assets + "entity/bed/green.png"),
	#("entity/bed/light_blue.png", assets + "entity/bed/light_blue.png"),
	#("entity/bed/lime.png", assets + "entity/bed/lime.png"),
	#("entity/bed/magenta.png", assets + "entity/bed/magenta.png"),
	#("entity/bed/orange.png", assets + "entity/bed/orange.png"),
	#("entity/bed/pink.png", assets + "entity/bed/pink.png"),
	#("entity/bed/purple.png", assets + "entity/bed/purple.png"),
	#("entity/bed/red.png", assets + "entity/bed/red.png"),
	#("entity/bed/silver.png", assets + "entity/bed/silver.png"),
	#("entity/bed/light_gray.png", assets + "entity/bed/light_gray.png"),
	#("entity/bed/white.png", assets + "entity/bed/white.png"),
	#("entity/bed/yellow.png", assets + "entity/bed/yellow.png"),
]

def has_imagemagick():
	try:
		# try to call convert command
		subprocess.check_output("convert")
		return True
	except subprocess.CalledProcessError:
		# command exited with error status, probably because we didn't specify any files to convert
		return True
	except OSError as e:
		# return False if command not found
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
			# unpack only PNGs, no other files (or directory entries)
			if not filename.endswith(".png"):
				continue
			# make sure to not unpack subdirectories
			base_path = os.path.dirname(filename)
			if base_path != os.path.dirname("blocks/test.png"):
				continue

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
		filename = os.path.join(args["outdir"], filename)
		try:
			info = jar.getinfo(zipname)
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
				subprocess.check_call(["convert", "-strip", filename, filename])
		
		filename = os.path.join(args["outdir"], "blocks", "red_sand.png")
		if os.path.exists(filename):
			subprocess.check_call(["convert", "-strip", filename, filename])
		
		filename = os.path.join(args["outdir"], "blocks", "glass_pane_top_white.png")
		if os.path.exists(filename):
			subprocess.check_call(["convert", "-strip", filename, "-type", "TrueColorMatte", "-define", "png:color-type=6", filename])
