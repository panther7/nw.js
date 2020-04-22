#!/bin/bash

set -e

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

. ${SCRIPTDIR}/version.sh

SEDCOMMANDE=(-rn)
if [[ $OSTYPE == "darwin"* ]]; then
    SEDCOMMANDE=(-En)
fi

RETVAL=0

GITTAG=$(git describe --tags | grep -oE "[^-]+$")

# e.g. NW.js v0.45.1+szn.1 (Node v13.12.0, Chromium 81.0.4044.92)
TITLE_MESSAGE="NW.js $GITTAG (Node v$NODE_VER, Chromium $CHROMIUM_VER)"
BODY_MESSAGE="${CI_COMMIT_SHORT_SHA}: ${CI_COMMIT_MESSAGE}
 - branch: ${CI_COMMIT_REF_NAME}
 - url: ${CI_PROJECT_URL}/-/commit/${CI_COMMIT_SHA}"

SRC_REPO=${PWD}

echo "Deploying from ${SRC_REPO}"

BIN_REPO=${HOME}/NWjs-build-linux
if [[ $OSTYPE == "darwin"* ]]; then
    BIN_REPO=${HOME}/NWjs-build-mac
fi

ARTIFACT=""

if [ -n "$1" ]; then
    while [ -n "$1" ]; do  # process ALL arguments
        case "$1" in
            *)
		if [[ -z ${SOURCE_FILE} ]]; then
                    ARTIFACT=${1%/}
		fi
		shift
		;;
        esac
    done

    if [[ -z ${ARTIFACT} ]]; then
        echo "No matching argument!"
        echo "Usage: [./deploy.sh <artifact>(like artifacts/sbrowser-v0.45.2+szn.1)]"
        exit 255
    fi

else
    echo "Usage: [./deploy.sh <artifact>(like artifacts/sbrowser-v0.45.2+szn.1)]"
    exit 255
fi

echo "Stepping into ${BIN_REPO}"

cd ${BIN_REPO}

RETVAL=$?
if [[ $RETVAL -ne 0 ]]; then
    echo "Directory does not exist"
    exit $RETVAL
fi

GITBRANCH=`git ls-remote --heads origin | cut -d '/' -f 3 | grep browser | tail -n 1` #browser45_dev
GITMAJOR=`echo ${GITBRANCH} | sed ${SEDCOMMANDE[@]} 's|browser([0-9]+)_dev|\1|p'`

git fetch -p --all
git checkout -B ${GITBRANCH} --track origin/${GITBRANCH}
git pull -f --rebase

if [[ ${ARTIFACT} == "/*" ]]; then
    cp ${ARTIFACT} ./
    ARTIFACT=$(basename ${ARTIFACT})
else
    echo "Copying ${SRC_REPO}/${ARTIFACT} -> ${PWD}"
    cp ${SRC_REPO}/${ARTIFACT} ./
fi

RETVAL=$?
if [[ $RETVAL -ne 0 ]]; then
    echo "Copying of ${ARTIFACT} has been unsuccesfull"
    exit $RETVAL
fi

NWJSTAG=`echo ${ARTIFACT} | sed ${SEDCOMMANDE[@]} 's|.*(v[0-9]+\.[0-9]+\.[0-9]+).*|\1|p'` #e.g. browser-v0.45.2+szn.1 = v0.45.2
NWJSMAJOR=`echo ${NWJSTAG} | sed ${SEDCOMMANDE[@]} 's|.*v[0-9]+\.([0-9]+)\.[0-9]+.*|\1|p'` #e.g. v0.45.2 = 45

if [[ -z ${GITMAJOR} ]]; then
    echo -e "\033[0;31m NWjs-build-* repository seems to be inconsistent. Please fix it manually. \033[0;37m"
    exit 1
fi

echo -e "\033[0;36mCompare ${GITMAJOR} -lt ${NWJSMAJOR}\033[0;37m"
if [[ ${GITMAJOR} -lt ${NWJSMAJOR} ]]; then
    GITBRANCH_OLD=${GITBRANCH}
    GITBRANCH=browser${NWJSMAJOR}_dev
    git checkout -B ${GITBRANCH}
    git branch -D ${GITBRANCH_OLD}
fi

echo "Removing previous versions..."

#remove previous versions
XARGSCOMMAND=(-d '\n')
if [[ $OSTYPE == "darwin"* ]]; then
    XARGSCOMMAND=(-n1)
fi

echo "Removing old versions in $PWD"

find ./ -maxdepth 1 -type d -print | grep -E "v[0-9]\.[0-9]+\.[0-9]+$" | xargs "${XARGSCOMMAND[@]}" sh -c 'for arg do echo "\033[0;95m removing: $arg\033[0;37m"; git rm -r --cached $arg; rm -rf $arg; done' _

echo "Unzipping ${ARTIFACT} ..."
tar xfz ${ARTIFACT}

RETVAL=$?
if [[ $RETVAL -ne 0 ]]; then
    echo "Unzipping ${ARTIFACT} has been unsuccesfull"
    exit $RETVAL
fi


echo "Removing tar ..."
rm -f ${ARTIFACT}

GITREMOTE=`git remote -v | grep fetch | cut -f 2 | cut -d' ' -f 1`
echo "Pushing into ${GITREMOTE}"

git add ${NWJSTAG}
git commit -m"${TITLE_MESSAGE}" -m"${BODY_MESSAGE}"
git push origin ${GITBRANCH}

echo -e "\033[0;32mDone uploading artifact!\033[0;37m"
