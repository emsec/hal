#!/usr/bin/env python3
import argparse
import sys
import os.path
from os import path
import re
from enum import Enum
import datetime

class release_info:

    def __init__(self):
        self.major = int()
        self.minor = int()
        self.patch = int()
        self.release = str()
        self.urgency = 'medium'
        self.description = []
        self.date = str()
        self.author = "Sebastian Wallat"
        self.email = "sebastian.wallat@rub.de"

    def __str__(self):
        return "entry: version: v{}.{}.{} release: {} urgency: {} date: {} author: {} email: {} description {}".format(self.major, self.minor, self.patch, self.release, self.urgency, self.date, self.author, self.email, self.description)

def to_debian(input, release = 'bionic'):
    token = [i.strip() for i in input.split("\n")]
    regex_start = r"\#\#\s+\[(?P<major>\d).(?P<minor>\d).(?P<patch>\d)\]\s+-\s+(?P<date>(?P<year>\d{4})-(?P<month>\d{2})-(?P<day>\d{2}) (\d{2}):(\d{2}):(\d{2})(\+|\-)(\d{2}):(\d{2}))\s*(\(urgency: (?P<urgency>low|medium|high)\))?"
    regex_hyperlink_section = r"\[//\]:\s+#\s+\(Hyperlink section\)"
    entries = []
    current_entry = None
    in_hyperlink_section = False

    for line in token:
        match_start = re.search(regex_start, line)
        match_hyperlink = re.search(regex_hyperlink_section, line)
        if match_start:
            if current_entry:
                entries.append(current_entry)
                current_entry = None
            current_entry = release_info()
            current_entry.major = match_start.group('major')
            current_entry.minor = match_start.group('minor')
            current_entry.patch = match_start.group('patch')
            current_entry.release = release
            current_entry.urgency = match_start.group('urgency')
            date_str = match_start.group('date')
            from dateutil import parser
            current_entry.date = parser.parse(date_str)

        elif match_hyperlink:
            in_hyperlink_section = True
            entries.append(current_entry)
            current_entry = None
        else:
            if current_entry != None and not in_hyperlink_section:
                current_entry.description.append(line)

    # Write output
    output_lines = []
    for entry in entries:
        if output_lines:
            output_lines.append("")
        output_lines.append("hal-reverse ({}.{}.{}) {}; urgency={}".format(entry.major, entry.minor, entry.patch, entry.release, entry.urgency))
        output_lines.append("")

        #Strip empty lines at begin and end of description
        if entry.description and entry.description[0] == "":
            entry.description.remove(entry.description[0])
        if entry.description and entry.description[-1] == "":
            entry.description.pop()

        output_lines += entry.description
        output_lines.append("")
        output_lines.append(" -- {} <{}> {}".format(entry.author, entry.email,entry.date))

    return "\n".join(output_lines)

class parse_debian_states(Enum):
    read_start = 1
    read_end = 2


def to_markdown(input):
    token = [i.strip() for i in input.split("\n")]
    regex_start = r"hal-reverse\s+\((?P<major>\d).(?P<minor>\d).(?P<patch>\d)\)\s+(?P<release>[\w]+);\s+urgency=(?P<urgency>[\w]+)"
    regex_end = r"\-\-\s+(?P<author>[\w ]+)\s+\<(?P<email>[\w.@]+)\>\s+(?P<date>(?P<day_of_week>Mon|Tue|Wed|Thu|Fri|Sat|Sun),\s+(?P<day>\d{2})\s+(?P<month>Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\s+(?P<year>\d{4})\s+(?P<hour>\d{2}):(?P<minute>\d{2}):(?P<second>\d{2})\s+(?P<time_shisht>(\+|\-)?\d{4}))"

    entries = []
    current_entry = None
    state = parse_debian_states.read_start
    for line in token:
        if state == parse_debian_states.read_start:
            match = re.search(regex_start, line)
            if match:
                current_entry = release_info()
                current_entry.major = match.group('major')
                current_entry.minor = match.group('minor')
                current_entry.patch = match.group('patch')
                current_entry.release = match.group('release')
                current_entry.urgency = match.group('urgency')
                state = parse_debian_states.read_end
        elif state == parse_debian_states.read_end:
            match = re.search(regex_end, line)
            if match:
                current_entry.author = match.group('author')
                current_entry.email = match.group('email')

                # Parse Timestamp
                date_str = match.group('date')
                from dateutil import parser
                current_entry.date = parser.parse(date_str)

                entries.append(current_entry)
                current_entry = None
                state = parse_debian_states.read_start
            else:
                if line != '':
                    current_entry.description.append(line)
    # Write output
    output_lines = []
    for entry in entries:
        if output_lines:
            output_lines.append("")
        line = "## [{}.{}.{}] - {}".format(entry.major, entry.minor, entry.patch, entry.date.isoformat(' '))

        if entry.urgency != 'medium':
            line = "{} (urgency: {})".format(line, entry.urgency)
        output_lines.append(line)
        output_lines.append("")
        output_lines += entry.description

    return "\n".join(output_lines)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert CHANGELOG from Markdown to Debian Syntax and vice versa.')
    parser.add_argument('input', type=str, nargs='?', help='Content of file to convert')
    parser.add_argument('-i', '--input-file', type=str, default='', help='Path to input file')
    parser.add_argument('--to', choices=['markdown', 'debian'], default='debian', help='Specify the desired format')
    parser.add_argument('-o', '--output-file', type=str, default="", help='Specify the output file name')
    parser.add_argument('--verbose', '-v', action='count')
    parser.add_argument('--force-write', '-f', action='store_true', default=False, help="Overwrite exiting file?")
    parser.add_argument('-r', '--release', type=str, default='bionic', help="Specify Ubuntu release to use!")

    args = parser.parse_args()
    input = ""
    if args.input_file:
        with open(args.input_file, 'r') as f:
            input = f.read()
    else:
        input = args.input

    result = ""
    if args.to == 'markdown':
        result = to_markdown(input)
    elif args.to == 'debian':
        result = to_debian(input, args.release)

    output_filename = args.output_file
    if output_filename != '':
        if path.exists(output_filename) and not args.force_write:
            print("Cannot overwrite existing file!", file=sys.stderr)
            exit(-1)
    if result:
        if output_filename != '':
            with open(output_filename, 'w+') as f:
                f.write(result)
        else:
            print(result)
    else:
        print("Skipping write out. No output produced!")




