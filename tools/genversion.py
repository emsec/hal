#!/usr/bin/env python3
import re
import subprocess
import shlex
import sys

def _execute_git_describe(cmd, regex_str):
    p = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    result, error = p.communicate()

    if error != None:
        return None

    result = result.decode('ascii').strip()
    # Use regex to extract specific parts to be passed to CMake
    return result, re.match(regex_str, result)


if __name__ == '__main__':
    # Use git describe to extract version information relative to the last tag
    commands = [
        "git describe --tags --match \"v[0-9]*\" --broken --dirty="" --always --abbrev=9",
        "git describe --tags --match \"v[0-9]*\" --dirty="" --always --abbrev=9"
        ]
    regex_strings = [
        "v((?P<major>\d+)\.(?P<minor>\d+)\.?(?P<patch>\d+)?(\.?(?P<tweak>\d+))?)(-(?P<add_commit>\d+))?(-g(?P<hash>[0-9,a-f]{9}))?(-(?P<dirty>dirty))?(-(?P<broken>broken))?",
        "v((?P<major>\d+)\.(?P<minor>\d+)\.?(?P<patch>\d+)?(\.?(?P<tweak>\d+))?)(-(?P<add_commit>\d+))?(-g(?P<hash>[0-9,a-f]{9}))?(-(?P<dirty>dirty))?"
        ]

    result, r = _execute_git_describe(commands[0], regex_strings[0])

    if r == None: result, r = _execute_git_describe(commands[1], regex_strings[1])

    if r == None:
        print("ERROR: could not call 'git describe'", file=sys.stderr)
        sys.exit(1)

    d = r.groupdict('0')

    # Boolean value is_dirty
    if 'dirty' not in d:
        d['dirty'] = "false"
    else:
        d['dirty'] = "true"

    # Boolean value is_broken
    if 'broken' not in d:
        d['broken'] = "false"
    else:
        d['broken'] = "true"

    ret_val = [result, d['major'], d['minor'], d['patch'], d['tweak'], d['add_commit'],  d['hash'], d['dirty'], d['broken']]

    print(";".join(ret_val))


