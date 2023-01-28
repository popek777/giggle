#!/bin/bash
#
rm -rf build/ CMakeUserPresets.json test_package/build

set -e
set -x

rm -rf build
mkdir build
pushd build

conan install .. --build=missing

conan build ..

conan export-pkg .. user/channel -f

# build (and run) test into explicit dir (-tbf)
conan test ../test_package giggle/0.1@user/channel
