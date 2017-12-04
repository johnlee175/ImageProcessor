#!/usr/bin/env bash

GPARAM='Unix Makefiles'
COMMANDS='make'

function print_usage
{
   cat << HEREDOC

   Usage: $progname [--gparam GPARAM] [--commands COMMAND]

   optional arguments:
     -h, -help, --help          show this help message and exit
     -v, -version, --version    show this version message and exit
     -G, --gparam GPARAM        provide param for "cmake -G"
     -C, --commands COMMANDS    provide command after cmake command done

HEREDOC
}

if [ $# -gt 0 ]; then
    for i in "$@"
    do
        case "$1" in
            -G|--gparam) GPARAM="$2"; shift 2;;
            -C|--commands) COMMANDS="$2"; shift 2;;

            --gparam=*) GPARAM="${1#*=}"; shift 1;;
            --commands=*) COMMANDS="${1#*=}"; shift 1;;

            -h|-help|--help) print_usage; exit; ;;
            -v|-version|--version) echo "Current-Version: 1.0.0"; exit; ;;
            "") shift 1;;
            *) echo -e "Unknown option: $1\nDo with --help for more details" >&2; exit 1; ;;
        esac
    done
fi

SHELL_DIR="$(cd "$(dirname "$0")"; pwd)"
BUILD_INFO="${SHELL_DIR}/../build.info.txt"

if [ -f "${BUILD_INFO}" ]; then
  while IFS='=' read -r key value
  do
    eval "${key}='${value}'"
  done < "${BUILD_INFO}"
else
  echo "${BUILD_INFO} not found."
  exit 1;
fi

BuildType="${BuildType:-debug}"
export BUILD_TYPE="$(tr '[:lower:]' '[:upper:]' <<< ${BuildType:0:1})${BuildType:1}"
#export CC=/path/to/your/c/compiler
#export CXX=/path/to/your/c++/compiler
export PROJECT='ImageProcessor'
export APP_VERSION="${BuildVersion:-1.0.0}"
export BUILD_DIR="${SHELL_DIR}/../${PROJECT}${BUILD_TYPE}-${APP_VERSION}-Build"
echo "BUILD_DIR='${BUILD_DIR}'"

rm -rf "${BUILD_DIR}" \
&& mkdir "${BUILD_DIR}" \
&& cd "${BUILD_DIR}" \
&& cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -G"${GPARAM}" "${SHELL_DIR}" \
&& ${COMMANDS}