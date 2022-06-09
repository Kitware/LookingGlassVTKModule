#!/usr/bin/env python3

import glob
import os
from pathlib import Path
import shutil
import subprocess
import sys

from utils import change_dir, unzip_file

"""
This script does the following:

1. Copy all vtk modules into the wheel
2. Run auditwheel repair
3. Remove anything that wasn't there before the repair

This is done so that we can get proper tags on the wheel, without the
RPATHs or contents of the wheel being affected.

Note: if we make the VTK modules visible to auditwheel by modifying the
LD_LIBRARY_PATH, auditwheel copies them into the project, but modifies
the RPATHs of our libraries. We do not want auditwheel to modify the
RPATHs, so we copy the VTK modules in ourselves.

It would be great if auditwheel, at some point, allowed us to add tags
to the wheel *without* repairing it...
"""

if len(sys.argv) < 3:
    sys.exit('Usage: <script> <wheel_path> <output_dir>')

wheel_path = sys.argv[1]
output_dir = sys.argv[2]

wheel_sdk_path = os.getenv('VTK_WHEEL_SDK_INSTALL_PATH')
if wheel_sdk_path is None:
    raise Exception('VTK_WHEEL_SDK_INSTALL_PATH must be set')

vtkmodules_glob_pattern = Path(wheel_sdk_path) / 'build/*/vtkmodules/*.so'
vtkmodules = glob.glob(str(vtkmodules_glob_pattern))
if not vtkmodules:
    raise Exception(f'Failed to find vtkmodules at: {vtkmodules_glob_pattern}')

with unzip_file(wheel_path) as unpacked_path:
    # Take a snapshot of the directory contents. After the wheel repair, we
    # will remove all files except for these.
    with change_dir(unpacked_path):
        contents_snapshot = list(Path().rglob('*'))

        # Find the path to the RECORD file
        record_glob = list(Path().glob('*.dist-info/RECORD'))
        if len(record_glob) != 1:
            raise Exception('Failed to find RECORD file')

        record_path = record_glob[0]

        # Read in the RECORD file. We will write it back out after the repair.
        with open(record_path, 'r') as rf:
            record_text = rf.read()

    # Copy the vtkmodules in
    destination = Path(unpacked_path) / 'vtkmodules'
    for f in vtkmodules:
        output = destination / Path(f).name
        if output.exists():
            # Don't overwrite any files
            continue

        shutil.copyfile(f, output)

# Now, perform the auditwheel repair
cmd = ['auditwheel', 'repair', wheel_path, '-w', output_dir]
subprocess.check_call(cmd)

# There should now be a wheel in the wheelhouse
output = glob.glob(f'{output_dir}/*.whl')
if len(output) != 1:
    raise Exception(f'Failed to find wheel in `{output_dir}`')

output_wheel = output[0]

# Now, remove all newly added files
with unzip_file(output_wheel) as unpacked_path:
    with change_dir(unpacked_path):
        for item in Path().rglob('*'):
            if item not in contents_snapshot:
                if item.is_dir():
                    shutil.rmtree(item)
                else:
                    item.unlink()

        # Replace the RECORD file with the old one
        with open(record_path, 'w') as wf:
            wf.write(record_text)
