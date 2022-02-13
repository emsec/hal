#!/usr/bin/env python3
import sys
import os

def print_usage():
    print("Doxygen Comment Checker")
    print("  (custom script, may contain errors)")
    print("")
    print("  Usage: python check_documentation.py inputpath1 [inputpath2 ...] [-i/-ignore ignorepath1 [ignorepath2]]")
    print("")
    print("  You can always provide directories or single files.")
    print("  All files/directories will be ignored which start with any of the ignorepaths.")
    print("")

##################################################################
##################################################################
##################################################################

# color codes for nice output formatting
class Colors:
    RED = '\033[91m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

# finds the start of the last block with given delimiters, with support for nested blocks.
# for example, given "ab () ( c ( de ) )", finding the last () block will return the position of the "(" before "c"
def index_of_last_block(text, block_start, block_end):
    last_end = len(text) - 1 - text[::-1].index(block_end)
    level = -1
    for i in range(last_end-1, -1, -1):
        if text[i] == block_end: level -= 1
        if text[i] == block_start:
            level += 1
            if level == 0:
                return i
    return None

# replaces a semantic block with given start/end delimiters by "#", e.g., everything between "<" and ">" becomes "#"
# this function is level-aware, i.e., it supports nested blocks: "xy<a<B>>" simply becomes "xy<####>" when removing the <> block
def blind_block(text, block_start, block_end):
    patches = ["operator<<", "operator>>", "operator[]", "operator()"]
    for i, p in enumerate(patches):
        text = text.replace(p, f"___PATCH##{i}##___")
    tmp = ""
    lvl = 0
    for c in text:
        if c == block_end and lvl > 0: lvl -= 1
        if lvl == 0:
            tmp += c
        else:
            tmp += "#"
        if c == block_start: lvl += 1
    for i, p in enumerate(patches):
        tmp = tmp.replace(f"___PATCH##{i}##___", p)
    return tmp

# analyze a scope, e.g., the global file scope, everything between { and }, or everything in a class
# this function is calling itself recursively on every inner scope it encounters
# returns a list of tuples (function signature, associated comment)
def analyze_scope_recursive(scope):
    candidates = list()

    in_public_part = True

    last_comment_block = None
    last_fragment_was_function = False

    while True:
        scope = scope.strip()

        # look for specific identifiers and narrow scope accordingly

        if len(scope) == 0: break

        if scope.startswith(("private:", "protected:")):
            in_public_part = False
            scope = scope[scope.index(":")+1:].strip()

        if scope.startswith("class"):
            in_public_part = False

        if (scope.startswith("public:")):
            in_public_part = True
            scope = scope[scope.index(":")+1:].strip()

        if not in_public_part:
            scope = scope[scope.index("\n")+1:] if ("\n" in scope) else ""
            continue

        if scope.startswith(("#", "//", "class", "struct", "namespace", "typedef", "using")):
            scope = scope[scope.index("\n")+1:] if ("\n" in scope) else ""
            continue

        # reached this point? potentially found a function!

        # now "blind" the scope, i.e., replace all template/function parameters with "#"
        blinded_scope = blind_block(scope, "(", ")")
        if "(" in blinded_scope:
            blinded_scope = blind_block(blinded_scope[:blinded_scope.index("(")], "<", ">") + blinded_scope[blinded_scope.index("("):]

        # find the index of the next comment block, the next "{" and the next ";"
        # whatever comes first determines how to continue
        comment_block_index = blinded_scope.index("/*") if ("/*" in blinded_scope) else len(scope) + 1
        bracket_index = blinded_scope.index("{") if ("{" in blinded_scope) else len(scope) + 1
        semicolon_index = blinded_scope.index(";") if (";" in blinded_scope) else len(scope) + 1

        next_index = min(comment_block_index,bracket_index,semicolon_index)

        # scope started with none of them
        if comment_block_index != 0 and bracket_index != 0 and semicolon_index != 0:
            # extract code until whichever came first
            blinded_code = blinded_scope[:next_index]
            code = scope[:next_index].replace("\n", " ")
            if "<" in blinded_code and ">" in blinded_code:
                blinded_code = blind_block(blinded_code, "<", ">")

            last_fragment_was_function = False

            # check that we potentially have a function
            if "(" in blinded_code and ")" in blinded_code and not blinded_code.endswith("= default") and not ":" in blinded_code.replace("::","#"):

                if len(blinded_code[:blinded_code.find("(")].strip().split()) >= 2: # this helps to ignore macro calls

                    # candidate for a function signature found
                    candidates.append((code, last_comment_block))
                    last_fragment_was_function = True

            # adjust scope
            scope = scope[next_index:]
            last_comment_block = None

        ### at this point, the scope definitely started with one of the three blocks

        # if it started with a comment block, extract the whole comment
        elif comment_block_index == next_index:
            end = scope.index("*/")+3
            last_comment_block = scope[:end]
            last_fragment_was_function = False
            scope = scope[end:]

        # if it started with an opening curly brace, we are at the start of a new scope --> recursive call
        elif bracket_index == next_index:
            level = 0
            for i in range(len(scope)):
                if scope[i] == "{": level += 1
                if scope[i] == "}": level -= 1
                if level == 0:
                    end = i
                    break
            if not last_fragment_was_function:
                candidates += analyze_scope_recursive(scope[1:end])
            scope = scope[end+1:]
            last_comment_block = None
            last_fragment_was_function = False

        # if it started with a semicolon, the current statement is simply over. continue with the next one
        elif semicolon_index == next_index:
            scope = scope[1:]
            last_comment_block = None
            last_fragment_was_function = False

    return candidates

# processes a file and returns tuples (function signature, associated comment)
def extract_functions(file):

    # load file as one long string
    with open(file) as f:
        content = "\n".join(x.strip() for x in f.readlines()).replace("\\\n", "")

    # analyze the global file scope (recursively analyzes subscopes)
    return analyze_scope_recursive(content)

# extract the parameters/arguments from a function
def extract_params(line):
    if "<" in line and ">" in line:
        line = blind_block(line.replace("operator()",""), "<", ">")
    line = line.replace("&", "")

    param_str = line[index_of_last_block(line,"(",")")+1:]
    param_str = param_str[:len(param_str)-1-param_str[::-1].index(")")]

    params = []

    for p in param_str.split(","):
        if p == "":
            continue
        if "=" in p:
            p = p[:p.index("=")]
        p = p.split()[-1].replace("[","").replace("]","").strip()
        params.append(p)
    return params

# extract the return type of a function
def extract_return_type(line):
    # ignore reserved keywords
    for x in ["explicit", "static", "typedef", "inline", "virtual"]:
        line = line.replace(x,"")

    line = blind_block(line[:index_of_last_block(line,"(",")")],"<",">").strip()

    # no space before "(" --> we have a constructor
    if ' ' not in line:
        return None

    line = line[:len(line)-1-line[::-1].index(' ')]

    return line.split()[-1]

# extract all commented parameters and there directions from a doxygen comment
def extract_commented_params(comment):
    if comment == None:
        return []
    params = []
    while "@param" in comment:
        comment = comment[comment.index("@param"):]
        indicator = comment[:comment.index(" ")]
        indicator = "[in]" in indicator or "[out]" in indicator or "[in,out]" in indicator
        comment = comment[comment.index(" ")+1 :]
        params.append((indicator,comment[:comment.index(" ")]))
    return params

# extract the comment describing the return value
def extract_return_type_comment(comment):
    if comment == None:
        return None
    if "@return" in comment:
        comment = blind_block(comment[comment.index("@return"):], "<",">")
        return comment.split()[1]
    return None

# analyze a single file
def analyze_file(file):
    print("===== " + file)
    error = False

    # check all functions of the file
    for (text, comment) in extract_functions(file):

        if comment and "@copydoc" in comment: continue

        params = extract_params(text)
        return_type = extract_return_type(text)
        commented_params = extract_commented_params(comment)
        return_type_comment = extract_return_type_comment(comment)

        not_commented = []
        type_missing = []
        commented_but_missing = []

        # check all commented params
        for param_tup in commented_params:
            # remember missing directions
            if not param_tup[0]:
                type_missing.append(param_tup[1])

            if param_tup[1] in params:
                params.remove(param_tup[1])
            else:
                commented_but_missing.append(param_tup[1])

        for param in params:
            not_commented.append(param)


        # consolidate booleans
        valid_return_types = [None, "void", "operator"]
        wrong_commenting = len(not_commented) > 0 or len(type_missing) > 0 or len(commented_but_missing) > 0
        return_type_problem = ((return_type in valid_return_types) and return_type_comment != None) or ((return_type not in valid_return_types) and return_type_comment == None) or (return_type_comment != None and return_type == return_type_comment)
        not_commented_error = comment == None and (return_type != None or len(params) > 0)

        # evaluate error cases
        if not_commented_error or wrong_commenting or return_type_problem:
            error = True
            print("  "+text)
            if not_commented_error:
                print(Colors.RED + "    the function is not commented."+ Colors.ENDC)
            else:
                for x in commented_but_missing:
                    print(Colors.RED + "    \"" + x + "\" is commented but not specified."+ Colors.ENDC)
                for x in type_missing:
                    print(Colors.RED + "    \"" + x + "\" is of unknown direction ([in], [out] or [in,out])."+ Colors.ENDC)
                for x in not_commented:
                    print(Colors.RED + "    \"" + x + "\" is specified but not commented."+ Colors.ENDC)
                if return_type == None and return_type_comment != None:
                    print(Colors.RED + "    function has no return type but one was commented."+ Colors.ENDC)
                if return_type not in valid_return_types and return_type_comment == None:
                    print(Colors.RED + "    function has a return type but none was commented."+ Colors.ENDC)
                if return_type_comment != None and return_type == return_type_comment:
                    print(Colors.RED + "    comment for return value is just the return type."+ Colors.ENDC)
            print("")

    return not error

# recursively check all files in a directory except the ones that start with an element of the ignore list
def check_recursive(thing, ignore_list):
    if any(thing.startswith(x) for x in ignore_list):
        return True
    if os.path.isfile(thing) and any(thing.endswith(x) for x in [".h", ".hpp", ".h.in"]):
        return analyze_file(thing)
    else:
        if not os.path.isdir(thing):
            return True # Current file is not a directory. Skip it
        if (thing[-1] != "/"):
            thing += "/"
        result = True
        for file in os.listdir(thing):
            result = check_recursive(thing + file, ignore_list) and result
        return result

##################################################################
##################################################################
##################################################################

if __name__ == '__main__':
    # parameter check
    if len(sys.argv) < 2:
        print_usage()
        sys.exit(1)

    # parse the command line arguments
    ignore_list = []
    directory_list = []
    do_ignore = False
    for i in range(1, len(sys.argv)):
        if sys.argv[i] == "-i" or sys.argv[i] == "-ignore":
            do_ignore = True
            continue
        if do_ignore:
            ignore_list.append(sys.argv[i])
        else:
            directory_list.append(sys.argv[i])

    # process the inputs
    all_ok = True
    for x in directory_list:
        all_ok = check_recursive(x, ignore_list) and all_ok

    # print the results and exit
    print("=====")
    if all_ok:
        print("Documentation correct.")
        sys.exit(0)
    else:
        print("Documentation errors were found.")
        sys.exit(1)
