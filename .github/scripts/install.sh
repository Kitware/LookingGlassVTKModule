#!/usr/bin/env bash
set -ev

#### Install cibuildwheel ####
pip install cibuildwheel

if [[ $RUNNER_OS == "Windows" ]]; then
    # Install wget (needed to download the SDK)
    choco install wget -y
elif [[ $RUNNER_OS == "macOS" ]]; then
    # VTK is expecting the xcode path to be slightly different.
    # Specifically, the VTK::RenderingOpenGL2 imported target seems
    # to expect the absolute path to the OpenGL library to match.
    ln -s /Applications/Xcode_13.1.app/ /Applications/Xcode-13.1.app
fi

#### Tell cibuildwheel where it will find the wheel sdk ####
if [[ $RUNNER_OS == "Linux" ]]; then
    # In docker on Linux, everything will be under /project instead of $PWD
    echo "VTK_WHEEL_SDK_PATH=/project/_deps/vtk-wheel-sdk" >> $GITHUB_ENV
else
    # The host is kept the same on Mac and Windows
    echo "VTK_WHEEL_SDK_PATH=$PWD/_deps/vtk-wheel-sdk" >> $GITHUB_ENV
fi
