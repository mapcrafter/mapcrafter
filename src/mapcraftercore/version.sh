#!/bin/sh

mc_version=$(cat ../../MCVERSION)
version=$(cat ../../VERSION)
gitversion=""
if [ -d "../../.git" ]; then
    gitversion=$(git describe)
fi

cat > version.cpp.tmp <<EOF
#include "version.h"

namespace mapcrafter {
	const char* MINECRAFT_VERSION = "$mc_version";
	const char* MAPCRAFTER_VERSION = "$version";
	const char* MAPCRAFTER_GITVERSION = "$gitversion";
};
EOF

if [ -f version.cpp ]; then
    if diff version.cpp.tmp version.cpp > /dev/null; then
        rm version.cpp.tmp
        exit
    fi
fi

mv version.cpp.tmp version.cpp
