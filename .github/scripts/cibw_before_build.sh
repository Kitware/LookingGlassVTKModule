#!/usr/bin/env bash
set -ev

if [[ $RUNNER_OS == "Linux" ]]; then
    # Some of the VTK modules use this library, and auditwheel will
    # complain if it can't find it. We will remove it from the wheel
    # after the repair is complete.
    yum install libXcursor-devel -y
fi
