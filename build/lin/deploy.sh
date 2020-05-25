#!/bin/bash

SEDCOMMAND="sed -rn"
if [[ $OSTYPE == "darwin"* ]]; then
    SEDCOMMAND="sed -En"
fi

RETVAL=0

GITTAG=`git describe --tags`
TITLE_MESSAGE=`git tag ${GITTAG} -n1 | cut -d' ' -f 2-`
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

GITHASH=`git rev-parse HEAD`
GITBRANCH=`git ls-remote --heads origin | grep ${GITHASH} | cut -d '/' -f 3` #browser45_dev
GITMAJOR=`echo ${GITBRANCH} | ${SEDCOMMAND} 's|browser([0-9]+)_dev|\1|p'`

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

NWJSTAG=`echo ${ARTIFACT} | ${SEDCOMMAND} 's|.*(v[0-9]+\.[0-9]+\.[0-9]+).*|\1|p'` #e.g. browser-v0.45.2+szn.1 = v0.45.2
NWJSMAJOR=`echo ${NWJSTAG} | ${SEDCOMMAND} 's|.*v[0-9]+\.([0-9]+)\.[0-9]+.*|\1|p'` #e.g. v0.45.2 = 45


echo "Compare ${GITMAJOR} -lt ${NWJSMAJOR}"
if [[ ${GITMAJOR} -lt ${NWJSMAJOR} ]]; then
    GITBRANCH=browser${NWJSMAJOR}_dev
    git checkout -b ${GITBRANCH}
fi

echo "Removing previous versions..."

#remove previous versions

XARGSCOMMAND="xargs -d $'\n'"
if [[ $OSTYPE == "darwin"* ]]; then
    XARGSCOMMAND="xargs -n1"
fi
find ./ -maxdepth 1 -type d -print | grep -E "v[0-9]\.[0-9]+\.[0-9]+$" | ${XARGSCOMMAND} sh -c 'for arg do git rm -r --cached $arg; rm -rf $arg; done' _

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
