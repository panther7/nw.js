#!/bin/bash

set -eo pipefail

# the directory this script resides no matter where it is called from
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

TARGET_NAME=""
SOURCE_DIR=""
OUT_DIR=""
WIDEVINE_DIR=${DIR}/WidevineCdm

if [ -n "$1" ]; then
    while [ -n "$1" ]; do  # process ALL arguments
        case "$1" in
	    -t|--target)
		TARGET_NAME="$2"
		shift
		shift
		;;
	    -d|--destination)
		OUT_DIR="$2"
		shift
		shift
		;;
            *)
		if [[ -z ${SOURCE_DIR} ]]; then
                    SOURCE_DIR=${1%/}
		fi
		shift
		;;
        esac
    done

    if [ -z "$SOURCE_DIR" ]; then
        echo "No matching argument!"
        echo "Usage: [./cp_nwjs_binaries.sh <dest_src>(like nwjs/src/out/nw) -d|--destionation <artifact_path> -t|--target <build_name>]"
        exit 255
    fi

else
    echo "Usage: [./cp_nwjs_binaries.sh <dest_src>(like nwjs/src/out/nw) -d|--destionation <artifact_path> -t|--target <build_name>]"
    exit 255
fi


SEDCOMMANDE=(-rn)
if [[ $OSTYPE == "darwin"* ]]; then
    SEDCOMMANDE=(-En)
fi

cd $SOURCE_DIR
#get the last tag of browser-v0.xy.wv+szn.z form and pick `v0.xy.vw`
LATEST_VERSION=`echo ${TARGET_NAME} | sed ${SEDCOMMANDE[@]} 's|.*(v[0-9]+\.[0-9]+\.[0-9]+).*|\1|p'`

#'s/.*\(v[0-9]\+\.[0-9]\+\.[0-9]\+\).*/\1/p'`
cd -

#if there is no artifact path specified, create some
if [[ -z ${OUT_DIR} ]]; then
    OUT_DIR=${HOME}/artifacts
fi

OUT_DIR=${OUT_DIR}/${LATEST_VERSION}
if [ ! -d "${OUT_DIR}" ]; then
    mkdir -p ${OUT_DIR}
fi

LIST_OF_NWJS_BIN_FILES=""

if [[ $OSTYPE == "msys" ]]; then
LIST_OF_NWJS_BIN_FILES="locales \
    swiftshader \
    icudtl.dat \
    nw.exe \
    notification_helper.exe \
    nw_100_percent.pak \
    nw_200_percent.pak \
    resources.pak \
    snapshot_blob.bin \
    v8_context_snapshot.bin \
    d3dcompiler_47.dll \
    ffmpeg.dll \
    libEGL.dll \
    libGLESv2.dll \
    node.dll \
    nw.dll \
    nw_elf.dll
"
elif [[ $OSTYPE == "darwin"* ]]; then
    LIST_OF_NWJS_BIN_FILES="nwjs.app"
else
LIST_OF_NWJS_BIN_FILES="lib \
    locales \
    icudtl.dat \
    nw \
    nw_100_percent.pak \
    nw_200_percent.pak \
    resources.pak \
    snapshot_blob.bin \
    v8_context_snapshot.bin
"
fi


echo "Copying into ${OUT_DIR}"
for file in $LIST_OF_NWJS_BIN_FILES; do
    cp -af $SOURCE_DIR/$file ${OUT_DIR}
done

if [[ $OSTYPE == "darwin"* ]]; then
    cp -af ${WIDEVINE_DIR} "${OUT_DIR}/nwjs.app/Contents/Frameworks/nwjs Framework.framework/Libraries/"
else
    cp -af ${WIDEVINE_DIR} ${OUT_DIR}
fi

echo "Zipping... ${LATEST_VERSION} to ${TARGET_NAME}.tar.gz (in ${OUT_DIR}/..)"
cd ${OUT_DIR}/..
tar cfz ${TARGET_NAME}.tar.gz ${LATEST_VERSION} #create zipped artifact

rm -rf ${LATEST_VERSION} #get rid of temporary directory

echo -e "\033[0;32mDone creating the artifact!\033]0;37m"
