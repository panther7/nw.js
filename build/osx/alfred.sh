#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
NW="master";
HANS_REMOTE_NAME="hans";
HANS=(
    "src https://github.com/janRucka/chromium.src-1.git browser${NW}-mac"
    "src/content/nw https://gitlab.seznam.net/sbrowser/software/core-chromium browser${NW}"
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
function task_clone () {
    local PIDS=();
    clone "src/content/nw" "https://github.com/nwjs/nw.js" "${NW}" & PIDS+=($!);
    clone "src/third_party/node-nw" "https://github.com/nwjs/node" "${NW}" & PIDS+=($!);
    clone "src/v8" "https://github.com/nwjs/v8" "${NW}" & PIDS+=($!);

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

EOF
    ) && echo "DONE! DONE! DONE!!!" || error "BUILD: FAIL! FAIL! FAIL!!!";
}

DO_TASK_CLONE=1;
DO_TASK_CONFIGURE=1;
DO_TASK_BUILD=1;

for arg in "$@"; do
    case "$arg" in
        "-?"|"-h"|"--help")
            echo "Availible tasks: clone, configure, build";
            echo "";
            echo "You can skip task using: --no-<task>";
            echo "    For example: --no-clone";
            exit 1;
        ;;
        "--no-clone")
            DO_TASK_CLONE=0
        ;;

        "--no-configure")
            DO_TASK_CONFIGURE=0
        ;;

        "--no-build")
            DO_TASK_BUILD=0
        ;;
    esac
done;

DEPOT_TOOLS="$/Users/fik/depot_tools";
if [ "$?" -gt 0 ]; then
	export PATH="$DEPOT_TOOLS:$PATH";
	echo "Added DEPOT_TOOLS to PATH: $PATH";
    gclient --sync;
else
	echo "Not adding DEPOT_TOOLS to PATH, because they seem to be already in :)"
    gclient --sync;
fi;

if [ "${DO_TASK_CLONE}" -gt 0 ]; then task_clone; fi;
if [ "${DO_TASK_CONFIGURE}" -gt 0 ]; then task_configure; fi;
if [ "${DO_TASK_BUILD}" -gt 0 ]; then task_build; fi;
echo DONE;
