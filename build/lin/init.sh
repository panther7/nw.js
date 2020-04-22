#!/bin/bash

echo "Let's initialize..."
echo "... in ${PWD}"

# the directory this script resides no matter where it is called from
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

#gitlab-ci clones repos under generated tokens, which needs to be replaced in order to satisfy gclient
git remote set-url origin git@gitlab.seznam.net:sbrowser/software/core-nw.git

SEDCOMMANDE=(-rn)
SEDCOMMANDI=(-i)
if [[ $OSTYPE == "darwin"* ]]; then
    SEDCOMMANDE=(-En)
    SEDCOMMANDI=(-i '' -e) # the f***g space between `-i' and '' is important
fi

GITTAG=`git describe --tags`
echo -e "\033[0;36mtag: \033[0;37m${GITTAG}"
GITHASH=`git rev-parse HEAD`
GITBRANCH=`git ls-remote --heads origin | grep ${GITHASH} | cut -d '/' -f 3`
echo -e "\033[0;36mbranch: \033[0;37m${GITBRANCH}"
NWJSMAJOR=`echo ${GITTAG} | sed "${SEDCOMMANDE[@]}" 's|.*v[0-9]+\.([0-9]+)\.[0-9]+.*|\1|p'` #e.g v0.45.2 = 45
echo -e "\033[0;36mversion: \033[0;37m${NWJSMAJOR}"
GITREMOTE=`git remote -v | grep fetch | cut -f 2 | cut -d' ' -f 1`
echo -e "\033[0;36mremote: \033[0;37m${GITREMOTE}"

echo "Fetching..."
git fetch --depth=1
git checkout --track origin/${GITBRANCH} 2>/dev/null #make sure it's tracking the upstream branch for gclient's sake

cd ../.. #get on top of `content/nw'
CHROMIUMSRC=${PWD}
echo "Entering updated source..."

if [[ -d ".git" ]]; then
    git reset --hard
fi

#cloning is a MUST, becuase depot_tools search for .git/index in it
echo "Cloning prerequisities..."
if [ ! -d "v8" ]; then
    git clone https://github.com/nwjs/v8
else
    git fetch -p --all
fi
cd v8

git checkout origin/nw${NWJSMAJOR}

cd ..
if [[ ! -d third_party ]]; then
    mkdir -p third_party
fi

cd third_party
if [ ! -d "node-nw" ]; then
    git clone https://github.com/nwjs/node node-nw
else
    git fetch -p --all
fi
cd node-nw
git checkout origin/nw${NWJSMAJOR}

cd ${CHROMIUMSRC}/..
NWJSSRC=${PWD}

echo "Copying gclient config from ${SCRIPTDIR}/.gclient ..."
cp ${SCRIPTDIR}/.gclient ./ #config of gclient

#echo "Reset gclient config on top of source..."
#sed -i -e "s|{1}|${GITBRANCH}|g" ./.gclient #replace {1} with current branch to sync onto

echo "Restore patches if they exist..." #this is needed as long as gclient scripts fail over that
if [ -d "${CHROMIUMSRC}/third_party/webdriver/pylib" ]; then
    cd  ${CHROMIUMSRC}/third_party/webdriver/pylib
    git reset --hard
fi
if [ -d "${CHROMIUMSRC}/third_party/ffmpeg" ]; then
    cd ${CHROMIUMSRC}/third_party/ffmpeg
    git reset --hard
fi
if [ -d "${CHROMIUMSRC}/third_party/icu" ]; then
    cd ${CHROMIUMSRC}/third_party/icu
    git reset --hard
fi
if [ -d "${CHROMIUMSRC}/third_party/perfetto" ]; then
    cd ${CHROMIUMSRC}/third_party/perfetto
    git reset --hard
fi
if [ -d "${CHROMIUMSRC}/third_party/devtools-frontend/src" ]; then
    cd ${CHROMIUMSRC}/third_party/devtools-frontend/src
    git reset --hard
fi
if [ -d "${CHROMIUMSRC}/third_party/angle/third_party/vulkan-loader/src" ]; then
    cd ${CHROMIUMSRC}/third_party/angle/third_party/vulkan-loader/src
    git reset --hard
fi

# unlock possibly stuck git repos
find ${NWJSSRC} -name "index.lock" -delete

# gclient is running outside the `src' directory
cd ${NWJSSRC}
gclient sync --with_branch_heads #sync all remaining dependencies (and patch them)

# get back to the original commit
cd ${CHROMIUMSRC}
git fetch --depth=1
git checkout ${GITTAG}

echo "Creating build paths..."
mkdir -p out/nw out/Release #build directories and a sandpit for build configurations too

echo "Copying ${SCRIPTDIR}/args.gn to build destination..."
cp ${SCRIPTDIR}/args.gn out/nw/ #nwjs build config

# Google API key has to be located at runner's HOME path
sed "${SEDCOMMANDI[@]}" "s|{_GAK_}|$(cat ${HOME}/.google_api_key)|g" out/nw/args.gn


if [[ $OSTYPE == "darwin"* ]]; then
    sed "${SEDCOMMANDI[@]}" 's/Seznam\.cz/nwjs/g' chrome/app/chromium_strings.grd
    sed "${SEDCOMMANDI[@]}" 's/Seznam\.cz/nwjs/g' chrome/app/theme/chromium/BRANDING
fi

echo "Generating essential build files..."
gn gen out/nw #create ninja files needed to build nwjs

echo "Generating node gyp files..."
GYPS=""
if [[ $OSTYPE == "linux-gnu" ]]; then
    GYPS="host_arch=x64 target_arch=x64"
elif [[ $OSTYPE == "darwin"* ]]; then
    GYPS="host_arch=x64 target_arch=x64 nwjs_sdk=1 mac_breakpad=1 symbol_level=0 fastbuild=1 buildtype=Official"
fi

GYP_DEFINES=${GYPS} GYP_CHROMIUM_NO_ACTION=0 ./build/gyp_chromium -I third_party/node-nw/common.gypi -D building_nw=1 third_party/node-nw/node.gyp #create gyp files needed to build custom node

echo -e "\033[0;32mDone initialization! \033[0;37m"




