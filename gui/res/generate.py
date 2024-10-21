#
# Copyright (c) 2024 Analog Devices Inc.
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

"""
This script supervises resources/ sub-folder ensuring that all icon files
referenced in the source code are present and also checks if there are
no stray .png and .svg files in resources/icons/.
It uses glob patterns and regular expressions to find icon names.
If all files are present then script will generate partial XML to be pasted
into resources.qrc.
"""


def generate():
    import glob
    import re
    import os.path
    base = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(base)
    icons = {}
    missing = {}
    rules = [
        # ( ';-separated file patterns', ';-separated regexps')
        ('**/*.qss;**/*.qss.c', r'(?<=url\(:/).*?(?=\))'),
        ('**/*.html*',          r'(?<=src="qrc:/).*?(?=")'),
        ('**/*.ui',             r'(?<=>:/).*?(?=<)'),
        ('**/*.ui',             r'(?<=url\(:/).*?(?=\))'),
        ('**/*.cpp;**/*.cc',    r'(?<=":/).*?\.(?:svg|png)(?=")'),
        ('**/*.cpp;**/*.cc',    r'(?<=QIcon::fromTheme\(").*?(?=");icons/scopy-default/icons/%s.svg;icons/scopy-light/icons/%s.svg'),
        ('src/toolmenu.cpp',    r'(?<=<< ").*(?=");icons/scopy-default/icons/%s.svg;icons/scopy-light/icons/%s.svg'),
        ('CMakeLists.txt',      r'(?<=resources/).*?\.svg'),
    ]
    for gs, rs in rules:
        gs, rs = gs.split(';'), rs.split(';')
        r, fmts = rs[0], rs[1:] if len(rs) > 1 else ['%s']
        for g in gs:
            for fn in glob.glob(os.path.join(root, g), recursive=True):
                # print(fn)
                with open(fn, 'r') as f:
                    t = f.read()
                for m in re.findall(r, t):
                    if m.startswith('/'):
                        m = m[1:]
                    for fmt in fmts:
                        ic = fmt % m
                        if ic in icons and fn not in icons[ic]:
                            icons[ic].append(os.path.relpath(fn, root))
                        else:
                            icons[ic] = [os.path.relpath(fn, root)]
    for ic in icons:
        if not os.path.exists(os.path.join(root, 'resources', ic)):
            missing[ic] = icons[ic]
    for ic, srcs in missing.items():
        print('!! missing', ic, ':', ', '.join(srcs))
    if len(missing) == 0:
        for ic in sorted(icons.keys()):
            print('        <file>%s</file> <!-- %s -->' % (ic, ', '.join(icons[ic])))
    unneeded = []
    for g in ['icons/**/*.png', 'icons/**/*.svg']:
        for fn in glob.glob(os.path.join(base, g), recursive=True):
            if os.path.relpath(fn, base) not in icons:
                # os.remove(fn)
                unneeded.append(os.path.relpath(fn, root))
    for fn in sorted(unneeded):
        print('!! unneeded', fn)


generate()
