#! /usr/bin/env python3

##########################################################################
# MIT License
#
# Copyright (C) 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
# Author: Jesper K. Pedersen <jesper.pedersen@kdab.com>
#
# This file is part of KDToolBox (https://github.com/KDAB/KDToolBox).
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, ** and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next paragraph)
# shall be included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF ** CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
##########################################################################

import os
import re
import os.path
import argparse
import sys

dirty = False


def stripInitialSlash(path):
    if path and path.startswith("/"):
        path = path[1:]
    return path

# Returns true if the path is to be excluded from the search


def shouldExclude(root, path):
    if not args.excludes:
        return False  # No excludes provided

    assert root.startswith(args.root)
    root = stripInitialSlash(root[len(args.root):])

    if args.headerPrefix:
        assert root.startswith(args.headerPrefix)
        root = stripInitialSlash(root[len(args.headerPrefix):])

    return (path in args.excludes) or (root + "/" + path in args.excludes)


regexp = re.compile("\\s*(Q_OBJECT|Q_GADGET|Q_NAMESPACE)\\s*")
# Returns true if the header file provides contains a Q_OBJECT, Q_GADGET or Q_NAMESPACE macro


def hasMacro(fileName):
    f = open(fileName, "r", encoding="utf8")
    for line in f:
        if regexp.match(line):
            return True
    return False

# returns the matching .cpp file for the given .h file


def matchingCPPFile(root, fileName):
    assert root.startswith(args.root)
    root = stripInitialSlash(root[len(args.root):])

    if args.headerPrefix:
        assert root.startswith(args.headerPrefix)
        root = stripInitialSlash(root[len(args.headerPrefix):])

    if args.sourcePrefix:
        root = args.sourcePrefix + "/" + root

    return args.root + "/"  \
        + root + ("/" if root != "" else "") \
        + fileNameWithoutExtension(fileName) + ".cpp"


def fileNameWithoutExtension(fileName):
    return os.path.splitext(os.path.basename(fileName))[0]

# returns true if the specifies .cpp file already has the proper include


def cppHasMOCInclude(fileName):
    includeStatement = '#include "moc_%s.cpp"' % fileNameWithoutExtension(fileName)
    f = open(fileName, encoding="utf8")
    return includeStatement in f.read()


def processFile(root, fileName):
    global dirty
    macroFound = hasMacro(root+"/"+fileName)
    if args.verbose:
        print("Inspecting %s %s" % (root+"/"+fileName, "[Has Q_OBJECT / Q_GADGET / Q_NAMESPACE]" if macroFound else ""))

    if macroFound:
        cppFileName = matchingCPPFile(root, fileName)
        if args.verbose:
            print("  -> %s" % cppFileName)

        if not os.path.exists(cppFileName):
            if not args.quiet:
                print("file %s didn't exist (which might not be an error)" % cppFileName)
            return

        if not cppHasMOCInclude(cppFileName):
            dirty = True
            if args.dryRun:
                if not args.quiet:
                    print("Missing moc include file: %s" % cppFileName)
            else:
                if not args.quiet:
                    print("Updating %s" % cppFileName)
                f = open(cppFileName, "a", encoding="utf8")
                f.write('\n#include "moc_%s.cpp"\n' % fileNameWithoutExtension(cppFileName))


################################ MAIN #################################
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""Script to add inclusion of mocs to files recursively.
        The source files either need to be in the same directories as the header files or in parallel directories,
        where the root of the headers are specified using --header-prefix and the root of the sources are specified using --source-prefix.
        If either header-prefix or source-prefix is the current directory, then they may be omitted.""")
    parser.add_argument("--dry-run", "-n", dest="dryRun", action='store_true', help="only report files to be updated")
    parser.add_argument("--quiet", "-q", dest="quiet", action='store_true', help="suppress output")
    parser.add_argument("--verbose", "-v", dest="verbose", action='store_true')
    parser.add_argument("--header-prefix", metavar="directory", dest="headerPrefix",
                        help="This directory will be replaced with source-prefix when searching for matching source files")
    parser.add_argument("--source-prefix", metavar="directory", dest="sourcePrefix", help="see --header-prefix")
    parser.add_argument("--excludes", metavar="directory", dest="excludes", nargs="*",
                        help="directories to be excluded, might either be in the form of a directory name, e.g. 3rdparty or a partial directory prefix from the root, e.g 3rdparty/parser")
    parser.add_argument(dest="root", default=".", metavar="directory",
                        nargs="?", help="root directory for the operation")

    args = parser.parse_args()

    root = args.root
    if args.headerPrefix:
        root += "/" + args.headerPrefix

    path = os.walk(root)
    for root, directories, files in path:
        # Filter out directories specified in --exclude
        directories[:] = [d for d in directories if not shouldExclude(root, d)]

        for file in files:
            if file.endswith(".h") or file.endswith(".hpp"):
                processFile(root, file)

    if not args.quiet and not dirty:
        print("No changes needed")

    sys.exit(-1 if dirty else 0)
