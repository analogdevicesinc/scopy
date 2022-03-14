def generate():
    import glob
    import re
    import os.path
    base = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(base)
    icons = {}
    missing = {}
    gr = [
        ('**/*.qss;**/*.qss.c', r'(?<=url\(:/).*?(?=\))'),
        ('**/*.html*',          r'(?<=src="qrc:/).*?(?=")'),
        ('**/*.ui',             r'(?<=>:/).*?(?=<)|(?<=url\(:/).*?(?=\))'),
        ('**/*.cpp;**/*.cc',    r'(?<=":/).*?\.(?:svg|png)(?=")'),
        ('**/*.cpp;**/*.cc',    r'(?<=QIcon::fromTheme\(").*?(?=");icons/scopy-default/icons/%s.svg;icons/scopy-light/icons/%s.svg'),
        ('src/toolmenu.cpp',    r'(?<=<< ").*(?=");icons/scopy-default/icons/%s.svg;icons/scopy-light/icons/%s.svg'),
    ]
    for gs, rs in gr:
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
