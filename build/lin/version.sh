#!/bin/bash

echo "Built upon..."

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"


FILE_NW_VERSION="$SCRIPTDIR/../../src/nw_version.h"
FILE_NODE_VERSION="$SCRIPTDIR/../../../../third_party/node-nw/src/node_version.h"
FILE_CHROMIUM_VERSION="$SCRIPTDIR/../../../../chrome/VERSION"


NW_NODE_H_PATTERN="#define [NODE|NW]+_[MAJOR|MINOR|PATCH]+_VERSION"
CHROMIUM_VER_PATTERN="[MAJOR|MINOR|BUILD|PATCH]+="


plumb_version () {
    cat $1 | grep -E "$2" | grep -oE "[^ |=]+$" | xargs echo | sed -e 's| |\.|g'
}

NW_VER=$(plumb_version $FILE_NW_VERSION "$NW_NODE_H_PATTERN")
NODE_VER=$(plumb_version $FILE_NODE_VERSION "$NW_NODE_H_PATTERN")
CHROMIUM_VER=$(plumb_version $FILE_CHROMIUM_VERSION "$CHROMIUM_VER_PATTERN")

echo -e "\033[0;36mNWjs\033[0;37m: $NW_VER"
echo -e "\033[0;36mNode\033[0;37m: $NODE_VER"
echo -e "\033[0;36mChromium\033[0;37m: $CHROMIUM_VER"


