#!/usr/bin/env bash
set -ev

#### Install cibuildwheel ####
pip install cibuildwheel

if [[ $RUNNER_OS == "macOS" ]]; then
    # VTK is expecting the xcode path to be slightly different.
    # Specifically, the VTK::RenderingOpenGL2 imported target seems
    # to expect the absolute path to the OpenGL library to match.
    ln -s /Applications/Xcode_13.1.app/ /Applications/Xcode-13.1.app
fi
