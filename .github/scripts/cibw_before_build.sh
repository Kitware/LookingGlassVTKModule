#!/usr/bin/env bash
set -ev

# FIXME: this is currently only running for python 3.9 wheels
if [[ $RUNNER_OS == "Windows" ]]; then
    sdk_url=https://data.kitware.com/api/v1/file/6294ed244acac99f42543957/download
elif [[ $RUNNER_OS == "macOS" ]]; then
    sdk_url=https://data.kitware.com/api/v1/file/6294ed374acac99f4254396d/download
elif [[ $RUNNER_OS == "Linux" ]]; then
    # Need to install wget
    yum install wget -y
    sdk_url=https://data.kitware.com/api/v1/file/6294ed4c4acac99f425439a8/download
fi

# Install the VTK SDK for this particular python version and OS
# The VTK_WHEEL_SDK_PATH contains the path where the sdk should end up
sdk_path=$VTK_WHEEL_SDK_PATH
rm -rf $sdk_path
mkdir -p $sdk_path

wget -qO- $sdk_url | tar xJ -C $sdk_path --strip-components=1
