#! /usr/bin/python3

from argparse import ArgumentParser
from subprocess import check_output
import subprocess
import sys
import re
import os
from itertools import chain

devnull = os.open("/dev/null", os.ST_WRITE)

def get_defines(f):
    out = []
    x = check_output(("gcc", "-E", "-dM", f)).decode("utf-8").split("\n")
    for y in x:
        match = re.match("#define (SDL[A-Za-z0-9_]*)", y)
        if match and is_int_define(f, match.group(1)):
            out.append(match.group(1))
    return out

def is_int_define(f, c):
    df = open("testfile.cpp", "w")
    df.write(r"""#include <cstdio>
#include "{}"
int main(int argc, char **argv) {{
std::printf("%d\n", static_cast<int>({}));
return 0;
}}""".format(f, c))
    df.close()
    return subprocess.call(
            ("g++", "-Wall", "-c", "testfile.cpp"), 
            stdout = devnull, stderr = devnull) == 0

def get_enums(f):
    out = []
    x = check_output(("gcc", "-E", f)).decode("utf-8").split("\n")
    x = "\n".join([x for x in x if (not x.startswith("#")) and (x.strip())])
    for block in re.findall(r"^(?:typedef )?enum\s*\{(.+?)\}\s*[A-Za-z0-9_]+\s*;\s*$", x, re.M | re.DOTALL):
        vars = block.split(",")
        for var in vars:
            match = re.match(r"^\s*([a-zA-Z0-9_]+)\s*", var)
            if match is None:
                print("WARN: Unrecognised enum", var.strip(), file=sys.stderr)
            else:
                out.append(match.group(1))
    return out

def get_config_defines(f):
    df = open(f, "r")
    fc = df.read()
    df.close()
    return re.findall("^(?:#define |/\* #undef )(SDL_[a-zA-Z0-9_]+)", fc, re.M)



def main():
    p = ArgumentParser()
    p.add_argument("--config", default="/usr/include/SDL2/SDL_config.h")
    p.add_argument("header", nargs = "?", default="/usr/include/SDL2/SDL.h")
    args = p.parse_args()
    config_defines = set(get_config_defines(args.config))
    constants = set(chain(get_defines(args.header), get_enums(args.header)))
    for x in config_defines:
        constants.discard(x)
    print("""
    #include <node.h>
    #include <SDL2/SDL.h>

    void InitConstants(v8::Handle<v8::Object> target) {
    v8::HandleScope scope;

    """)
    for x in constants:
        x = x.strip()
        name = None
        if x.startswith("SDL_"):
            name = x[4:]
        elif x.startswith("SDLK"):
            name = x[3:]
        elif x.startswith("KMOD_"):
            name = x 
        if name:
            print('target->Set(v8::String::NewSymbol("', 
                    name, 
                    '"), v8::Integer::New(', 
                    x, 
                    '));', sep='')
    #Handle config_defines
    for x in config_defines:
        if x.find("DYNAMIC") != -1:
            #DYNAMIC are strings, not defs
            continue
        name = x[4:]
        print("#ifdef", x)
        print('target->Set(v8::String::NewSymbol("', 
                name, 
                '"), v8::Integer::New(', 
                x, 
                '));', sep='')
        print("#endif")
    print("}")

if __name__ == "__main__":
    main()

