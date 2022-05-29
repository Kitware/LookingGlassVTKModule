import os
from pathlib import Path
import shutil
import subprocess
import sys

from skbuild import setup

vtk_looking_glass_module_source_dir = Path(__file__).parent.resolve()

vtk_wheel_sdk_path = os.getenv('VTK_WHEEL_SDK_PATH')
if vtk_wheel_sdk_path is None:
    raise Exception('VTK_WHEEL_SDK_PATH has not been set')

# Find the cmake dir
cmake_glob = list(Path(vtk_wheel_sdk_path).glob('**/headers/cmake'))
if len(cmake_glob) != 1:
    raise Exception('Unable to find cmake directory')

vtk_wheel_sdk_cmake_path = cmake_glob[0]

vtk_external_module_path = os.getenv('VTK_EXTERNAL_MODULE_PATH')
if vtk_external_module_path is None:
    # If it was not provided, clone it into a temporary directory
    # Since we are using pyproject.toml, it will get removed automatically
    script_path = str(vtk_looking_glass_module_source_dir /
                      'FetchVTKExternalModule.cmake')

    external_module_path = Path('.').resolve() / '_deps/VTKExternalModule'
    cmd = [
        'cmake',
        '-DFETCH_VTKExternalModule_INSTALL_LOCATION=' +
        external_module_path.as_posix(),
        '-P', script_path,
    ]
    subprocess.check_call(cmd)
    vtk_external_module_path = external_module_path.as_posix()

python3_executable = os.getenv('Python3_EXECUTABLE')
if python3_executable is None:
    python3_executable = shutil.which('python')

if python3_executable is None:
    msg = 'Unable find python executable, please set Python3_EXECUTABLE'
    raise Exception(msg)

cmake_args = [
    '-DVTK_MODULE_NAME:STRING=RenderingLookingGlass',
    f'-DVTK_MODULE_SOURCE_DIR:PATH={vtk_looking_glass_module_source_dir}',
    f'-DVTK_MODULE_CMAKE_MODULE_PATH:PATH={vtk_wheel_sdk_cmake_path}',
    f'-DVTK_DIR:PATH={vtk_wheel_sdk_cmake_path}',
    '-DCMAKE_INSTALL_LIBDIR:STRING=lib',
    f'-DPython3_EXECUTABLE:FILEPATH={python3_executable}',
    '-DVTK_WHEEL_BUILD:BOOL=ON',
    '-S', vtk_external_module_path,
]

if sys.platform == 'linux':
    # We currently have to add this for the render window to get compiled
    cmake_args.append('-DVTK_USE_X:BOOL=ON')
elif sys.platform == 'darwin':
    # We currently have to add this for the render window to get compiled
    cmake_args.append('-DVTK_USE_COCOA:BOOL=ON')

setup(
    name='vtk-lookingglass',
    version='9.1.0',  # How do we version with VTK ?
    description='Looking Glass support for VTK Python.',
    long_description='Looking Glass support for VTK Python.',
    url='',
    author='Kitware Inc',
    license='MIT',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3',
    ],
    keywords='',
    packages=['vtkmodules'],
    package_dir={'vtkmodules': 'lib/vtkmodules'},
    cmake_args=cmake_args,
)
