#! /usr/bin/python
from __future__ import print_function
import os
import os.path
import shutil
import subprocess
import sys
from optparse import OptionParser

def cd_build():
    global build
    #Create build directory (if it does not exist)
    try:
        os.makedirs(build)
    except OSError:
        pass
    #Change to build directory
    os.chdir(build)

def configure(noprint=False):
    global configure_path
    cd_build()
    #Run configure
    environ = dict(os.environ)
    environ.update({"CFLAGS": "-fPIC"})
    cmd = (configure_path, "--prefix={}".format(prefix))
    kwargs = {"env": environ}
    if noprint:
        #Redirect to /dev/tty
        try:
            fd = os.open("/dev/tty", os.O_WRONLY)
            try:
                subprocess.check_call(cmd, stdout=fd, stderr=fd, **kwargs)
            except subprocess.CalledProcessError:
                clean()
                raise
        except OSError:
            #Try another method
            try:
                subprocess.check_output(cmd, stderr=subprocess.STDOUT, **kwargs)
            except subprocess.CalledProcessError as e:
                print(e.output, file=sys.stderr)
                clean()
                raise
    else:
        try:
            subprocess.check_call(cmd, **kwargs)
        except subprocess.CalledProcessError:
            clean()
            raise

def clean():
    global build
    os.chdir('/')
    shutil.rmtree(build, ignore_errors=True)

def main():
    global configure_path
    global prefix
    global build
    p = OptionParser()
    p.add_option("--dest", dest="dest", default="") #Set dest directory
    p.add_option("--build", dest="build", default=None)
    p.add_option("--libs", dest="libs", action="store_true", default=False)
    (options, args) = p.parse_args()
    prefix = os.path.abspath(options.dest)
    if not options.build:
        build = os.path.abspath("build")
    else:
        build = os.path.abspath(options.build)
    configure_path = os.path.abspath("SDL/configure") #Path to configure script
    if options.libs:
        configure(noprint=True)
        libs = subprocess.check_output(["sh", "sdl2-config", "--static-libs"]).split()
        libs = [x for x in libs if x.startswith("-l") and x != "-lSDL2"]
        print(" ".join(libs))
        clean()
    else:
        configure()
        subprocess.check_call(("make", "install"))

if __name__ == "__main__":
    main()
