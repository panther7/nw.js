#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
NW=27;
HANS_REMOTE_NAME="hans";
HANS=(
    "src https://github.com/janRucka/chromium.src-1.git browser${NW}-mac"
    "src/content/nw https://github.com/janRucka/nw.js.git browser${NW}"
);

export GYP_GENERATORS="ninja"
export GYP_DEFINES="host_arch=x64 target_arch=x64 nwjs_sdk=1 mac_breakpad=1 disable_nacl=1 debug=0 symbol_level=0 remove_webcore_debug_symbols=1 fastbuild=1 buildtype=Official enable_pepper_cdms=1 enable_webrtc=1"
export GYP_CHROMIUM_NO_ACTION=0

# Vypíše error a exitne s kódem 255
function error () {
    echo $@ >&2;
    exit 255;
}

# Vyklonuje repo a checkoutne větev
function clone () {
    local TARGET="${DIR}/${1}";
    local SRC="${2}";
    local BRANCH="${3}";

    echo "Clonning ${SRC}@${BRANCH} --> ${TARGET}...";
    mkdir -p "${TARGET}" && git clone --quiet -b "${BRANCH}" "${SRC}" "${TARGET}" || error "Failed to clone ${TARGET}";
    echo "Sucessfully cloned ${SRC}@${BRANCH} --> ${TARGET}"
}

# Ohansuje repo. Tzn přidá remote, fetchne a checkoutne větev
function hans () {
    local TARGET="${DIR}/${1}";
    local SRC="${2}";
    local BRANCH="${3}";

    echo "Hansing ${TARGET} --> ${SRC}@${BRANCH}...";
    (
        bash <<EOF
cd "${TARGET}" || exit 253;
for r in \$(git remote); do
    if [ "\$r" = "${HANS_REMOTE_NAME}" ]; then git remote remove "${HANS_REMOTE_NAME}" >/dev/null || exit 254; fi;
done;
git remote add "${HANS_REMOTE_NAME}" "${SRC}" &&
git fetch --quiet "${HANS_REMOTE_NAME}" &&
git checkout -B "${BRANCH}" --track "${HANS_REMOTE_NAME}/${BRANCH}" || exit 255
EOF
    )  || error "Failed to hans $TARGET";
    echo "Sucessfully Hansed ${TARGET} --> ${SRC}@${BRANCH}";
}

function task_clone () {
    local PIDS=();
    clone "src/content/nw" "https://github.com/nwjs/nw.js" "nw${NW}" & PIDS+=($!);
    clone "src/third_party/node-nw" "https://github.com/nwjs/node" "nw${NW}" & PIDS+=($!);
    clone "src/v8" "https://github.com/nwjs/v8" "nw${NW}" & PIDS+=($!);

    wait "${PIDS[@]}" || error "Failed to wait..." # Počkáme na ty 3 klony

    cat >"${DIR}/.gclient" <<EOF
solutions = [
  { "name"        : "src",
    "url"         : "https://github.com/nwjs/chromium.src.git@origin/nw${NW}",
    "deps_file"   : "DEPS",
    "managed"     : True,
    "custom_deps" : {
        "src/third_party/WebKit/LayoutTests": None,
        "src/chrome_frame/tools/test/reference_build/chrome": None,
        "src/chrome_frame/tools/test/reference_build/chrome_win": None,
        "src/chrome/tools/test/reference_build/chrome": None,
        "src/chrome/tools/test/reference_build/chrome_linux": None,
        "src/chrome/tools/test/reference_build/chrome_mac": None,
        "src/chrome/tools/test/reference_build/chrome_win": None,
    },
    "custom_vars": {},
  },
]
cache_dir = None
EOF

    gclient sync --with_branch_heads --nohooks;
}

function task_hans () {
    local PIDS=();
    for args in "${HANS[@]}"; do
        hans $args &
        PIDS+=($!);
    done;
    wait "${PIDS[@]}" || error "Failed to wait..." # Počkáme na ohansování
}

function task_configure () {
    local REQUIRED_XCODE="${DIR}/Xcode.app/Contents/Developer";
    if [ -d "${REQUIRED_XCODE}" ]; then
        local CURRENT_XCODE="$(xcode-select -p)";
        if [ "${CURRENT_XCODE}" != "${REQUIRED_XCODE}" ]; then
            echo "We need to switch XCode to '${REQUIRED_XCODE}'. You must be root to do so. Enter sudo password for '$(whoami)'";
            sudo xcode-select --switch "${REQUIRED_XCODE}" || error "Failed to switch Xcode";
        else
            echo "Current XCode are correct";
        fi;
    fi;

    local ENABLE_WIDEWINE="true";
    if [ "$(uname)" = "Darwin" ]; then
        # header must be manually swapped so widevine support is available,
        # but the widevine stub wil not build
        ENABLE_WIDEWINE="false";
    fi;
    (
        bash <<EOF;
cd "${DIR}" &&
gclient run-hooks &&
cd "${DIR}/src" &&
mkdir -p "out/nw" &&
(
    cat >out/nw/args.gn <<ARGS
google_api_key="AIzaSyDNm5jc9OlE3TEcRKaczAraY-II0XMGi6g"
is_debug=false
target_cpu="x64"
proprietary_codecs=true
is_component_ffmpeg=true
is_official_build=true
is_component_build=false
ffmpeg_branding="Chrome"
nwjs_sdk=true
enable_widevine=${ENABLE_WIDEWINE}

# Build Speedup
symbol_level=0

# disable [chromium-style] errors
clang_use_chrome_plugins=false

# Shrink size
remove_webcore_debug_symbols=true
ARGS
) &&
gn gen "out/nw" &&
./build/gyp_chromium -I third_party/node-nw/common.gypi -D building_nw=1 third_party/node-nw/node.gyp || exit 255;
EOF
    ) || error "Failed to configure";
}

function task_patch_widevine () {
    if [ "$(uname)" = "Darwin" ]; then
        (
            bash <<EOF;
cd "${DIR}/src/third_party/widevine/cdm" &&
git checkout -- . &&
mv widevine_cdm_common.h widevine_cdm_common.h.bak &&
grep -iv buildflag widevine_cdm_common.h.bak > widevine_cdm_common.h &&
cp stub/* . &&
echo '#define WIDEVINE_CDM_VERSION_STRING "1.4.8.1030"' >> widevine_cdm_version.h || exit 255
EOF
        ) || error "Failed to patch widevine";
    else
        echo "Not patching Widewine on this platform"
    fi;
}

function task_build () {
    (
        bash <<EOF
cd "${DIR}/src" &&
ninja -C out/nw nwjs -j8 &&
ninja -C out/Release node -j8 &&
ninja -C out/nw copy_node &&
echo "Main build DONE!!!" || exit 255;

VERSION_DIR=\$(find out/nw/nwjs.app/Contents/Versions -type d -depth 1);
echo "VERSION_DIR=\$VERSION_DIR";

if [ -d "\$VERSION_DIR" ]; then
	if [ -L "\$VERSION_DIR/nwjs Framework.framework/Libraries" ]; then
		rm "\$VERSION_DIR/nwjs Framework.framework/Libraries";
	fi;

	cp -XPR "../Libraries" "\$VERSION_DIR/nwjs Framework.framework/Versions/A" &&
	ln -s Versions/A/Libraries "\$VERSION_DIR/nwjs Framework.framework/Libraries" &&
	cp -XPR "../Widevine Resources.bundle" "\$VERSION_DIR" || exit 255;
else
	echo "Failed to locate VERSION_DIR";
	exit 255;
fi;

EOF
    ) && echo "DONE! DONE! DONE!!!" || error "BUILD: FAIL! FAIL! FAIL!!!";
}

DO_TASK_CLONE=1;
DO_TASK_HANS=1;
DO_TASK_CONFIGURE=1;
DO_TASK_PATCH_WIDEVINE=1;
DO_TASK_BUILD=1;

for arg in "$@"; do
    case "$arg" in
        "-?"|"-h"|"--help")
            echo "Availible tasks: clone, hans, configure, patch-widevine, build";
            echo "";
            echo "You can skip task using: --no-<task>";
            echo "    For example: --no-clone";
            exit 1;
        ;;
        "--no-clone")
            DO_TASK_CLONE=0
        ;;

        "--no-hans")
            DO_TASK_HANS=0
        ;;

        "--no-configure")
            DO_TASK_CONFIGURE=0
        ;;

        "--no-patch-widevine")
            DO_TASK_PATCH_WIDEVINE=0
        ;;

        "--no-build")
            DO_TASK_BUILD=0
        ;;
    esac
done;

DEPOT_TOOLS="$( dirname "$DIR" )/depot_tools";
echo "$PATH" | grep "${DEPOT_TOOLS}:" &>/dev/null;
if [ "$?" -gt 0 ]; then
	export PATH="$DEPOT_TOOLS:$PATH";
	echo "Added DEPOT_TOOLS to PATH: $PATH";
else
	echo "Not adding DEPOT_TOOLS to PATH, because they seem to be already in :)"
fi;

if [ "${DO_TASK_CLONE}" -gt 0 ]; then task_clone; fi;
if [ "${DO_TASK_HANS}" -gt 0 ]; then task_hans; fi;
if [ "${DO_TASK_CONFIGURE}" -gt 0 ]; then task_configure; fi;
if [ "${DO_TASK_PATCH_WIDEVINE}" -gt 0 ]; then task_patch_widevine; fi;
if [ "${DO_TASK_BUILD}" -gt 0 ]; then task_build; fi;
echo DONE;
