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
# Auto-format scopy files after editing.
# Triggered as a PostToolUse hook for Edit/Write/MultiEdit.

input=$(cat)
file_path=$(echo "$input" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('tool_input',{}).get('file_path',''))" 2>/dev/null)

# Only act on files under the scopy/ project
if [[ "$file_path" != *"/scopy/"* ]]; then
  exit 0
fi

case "$file_path" in
  *.cpp|*.cc|*.hpp|*.h)
    clang-format -i "$file_path" ;;
  */CMakeLists.txt|*.cmake)
    cmake-format -i "$file_path" ;;
esac

exit 0
