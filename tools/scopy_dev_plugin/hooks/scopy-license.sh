#!/bin/bash
#
# Copyright (c) 2025 Analog Devices Inc.
#
# This file is part of Scopy
# (see https://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
# Auto-add license header to new scopy files after the Write tool creates them.

input=$(cat)
file_path=$(echo "$input" | python3 -c \
  "import sys,json; d=json.load(sys.stdin); print(d.get('tool_input',{}).get('file_path',''))" \
  2>/dev/null)

# Only act on files under the scopy/ project
if [[ "$file_path" != *"/scopy/"* ]]; then
  exit 0
fi

# Only act on supported source file types
case "$file_path" in
  *.cpp|*.cc|*.hpp|*.h|*.js|*/CMakeLists.txt|*.cmake) ;;
  *) exit 0 ;;
esac

# Skip if file already has a license header
if head -80 "$file_path" 2>/dev/null | grep -q "Copyright"; then
  exit 0
fi

# Derive scopy repo root from this script's location
# Script is at scopy/tools/scopy_dev_plugin/hooks/scopy-license.sh
# So ../../../ is scopy/
SCOPY_REPO="$(cd "$(dirname "$0")/../../.." && pwd)"
LICENSE_UTILS="$SCOPY_REPO/tools/license-header"
TEMPLATE="$LICENSE_UTILS/templates/Scopy/LICENSE"
PARAMS="$LICENSE_UTILS/templates/Scopy/params.conf"

tmp_list=$(mktemp)
echo "$file_path" > "$tmp_list"

pushd "$LICENSE_UTILS" > /dev/null || exit 0
./batch_add_license_headers.sh "$tmp_list" --template "$TEMPLATE" --params "$PARAMS"
popd > /dev/null

rm -f "$tmp_list"
exit 0
