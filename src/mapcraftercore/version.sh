#!/bin/sh

mc_version=$(cat ../../MCVERSION | sed 's/\n//')
version=$(cat ../../VERSION | sed 's/\n//')
gitversion=""
if [ -d "../../.git" ]; then
    gitversion=$(git describe)
fi

echo "#include \"version.h\"" > version.cpp.tmp
echo "namespace mapcrafter {" >> version.cpp.tmp
echo "const char* MINECRAFT_VERSION = \"$mc_version\";" >> version.cpp.tmp
echo "const char* MAPCRAFTER_VERSION = \"$version\";" >> version.cpp.tmp
echo "const char* MAPCRAFTER_GITVERSION = \"$gitversion\";" >> version.cpp.tmp
echo "};" >> version.cpp.tmp

if [ -f version.cpp ]; then
    if diff version.cpp.tmp version.cpp > /dev/null; then
        rm version.cpp.tmp
        exit 0
    fi
fi

mv version.cpp.tmp version.cpp
