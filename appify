#!/bin/bash

# script: appify.sh
# minor edits from https://gist.github.com/advorak/1403124

# usage and cli args
usage="Creates the simplest possible Mac app from a shell script.\n\
Usage: `basename "$0"` -s <shell-script.sh> -n <app-name>\n\
[optional: -i <icon.icn> -h <help>]\n\
\n\
Copyright (c) Thomas Aylott <http://subtlegradient.com/>\n\
Modified by Mathias Bynens <http://mathiasbynens.be/>\n\
Modified by Andrew Dvorak <http://OhReally.net/>\n\
Modified by Stephane Plaisance (support for opt:arguments)"

while getopts "s:n:i:h" opt; do
  case $opt in
    s) script=${OPTARG} ;;
    n) name=${OPTARG} ;;
    i) icon=${OPTARG} ;;
    h) echo -e ${usage} >&2; exit 0 ;;
    \?) echo "Invalid option: -$OPTARG" >&2; exit 1 ;;
    *) echo "this command requires arguments, try -h" >&2; exit 1 ;;
  esac
done

# test minimal argument
if [ -z "${script}" ]; then
   echo "# no input script provided!"
   echo -e ${usage} >&2
   exit 1
fi

# test input files
if [ ! -f "${script}" ]; then
   echo "# ${script} file not found!" >&2
   exit 1
fi

# test optional args
if [ -z "${name}" ]; then
   APPNAME=$(basename "${script}" ".sh")
else
   APPNAME=${name}
fi

if [ -a "${APPNAME}.app" ]; then
    echo "# $PWD/${APPNAME}.app already exists, use -n <appname>!" >&2
    exit 1
fi

if [ -z "${icon}" ]; then
   # support for custom icon with default
   ICONFOLD="/System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/"
   CUSTICON="${ICONFOLD}/GenericApplicationIcon.icns"
else
   CUSTICON=${icon}
fi

if [ ! -f ${CUSTICON} ]; then
   echo "# ${CUSTICON} file not found!" >&2
   exit 1
fi

# define output path
DIR="${APPNAME}.app/Contents"
mkdir -p $DIR/{MacOS,Resources} || (echo "error creating the App container" >&2; exit 1);

# Copy Apple's GenericApplicationIcon to our application.
cp "${CUSTICON}" "${DIR}/Resources/${APPNAME}.icns"
cp "${script}" "${DIR}/MacOS/${APPNAME}"
chmod +x "${DIR}/MacOS/${APPNAME}"

cat <<EOF > $DIR/Info.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>$APPNAME</string>
    <key>CFBundleGetInfoString</key>
    <string>$APPNAME</string>
    <key>CFBundleIconFile</key>
    <string>$APPNAME</string>
    <key>CFBundleName</key>
    <string>$APPNAME</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
</dict>
</plist>
EOF

echo "$PWD/${APPNAME}.app was created"
