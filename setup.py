import os
import shutil
from pathlib import Path

from skbuild import setup

vtk_wheel_sdk_path=os.getenv('VTK_WHEEL_SDK_PATH')
if vtk_wheel_sdk_path is None:
    raise Exception('VTK_WHEEL_SDK_PATH has not been set')

# Find the cmake dir
cmake_glob = list(Path(vtk_wheel_sdk_path).glob('**/headers/cmake'))
if len(cmake_glob) != 1:
    raise Exception('Unable to find cmake directory')

vtk_wheel_sdk_cmake_path = cmake_glob[0]

holoplaycore_include_dir=os.getenv('HOLOPLAYCORE_INCLUDE_DIR')
if holoplaycore_include_dir is None:
    raise Exception('HOLOPLAYCORE_INCLUDE_DIR has not been set')

holoplaycore_library=os.getenv('HOLOPLAYCORE_LIBRARY')
if holoplaycore_library is None:
    raise Exception('HOLOPLAYCORE_LIBRARY has not been set')

python3_executable=os.getenv('PYTHON3_EXECUTABLE')
if python3_executable is None:
    python3_executable = shutil.which('python')

if python3_executable is None:
    raise Exception('Unable find python executable, please set PYTHON3_EXECUTABLE')

vtk_external_module_path=os.getenv('VTK_EXTERNAL_MODULE_PATH')
if vtk_external_module_path is None:
    raise Exception('VTK_EXTERNAL_MODULE_PATH has not been set')

lg_vtk_module_path = Path(__file__).parent.resolve()

cmake_args = [
  '-DVTK_MODULE_NAME:STRING=RenderingLookingGlass',
  f'-DVTK_MODULE_SOURCE_DIR:PATH={lg_vtk_module_path}',
  f'-DVTK_MODULE_CMAKE_MODULE_PATH:PATH={vtk_wheel_sdk_cmake_path}',
  f'-DVTK_DIR:PATH={vtk_wheel_sdk_cmake_path}',
  f'-DHoloPlayCore_INCLUDE_DIR:PATH={holoplaycore_include_dir}',
  f'-DHoloPlayCore_LIBRARY:FILEPATH={holoplaycore_library}',
  '-DCMAKE_INSTALL_LIBDIR:STRING=lib',
  f'-DPython3_EXECUTABLE:FILEPATH={python3_executable}',
  # TODO: Remove this ...
  '-DCMAKE_CXX_FLAGS=-D_GLIBCXX_USE_CXX11_ABI=0',
  '-DVTK_WHEEL_BUILD:BOOL=ON',
  '-S', vtk_external_module_path
]

setup(
    name='vtk-lg',
    version="9.1.0", # How do we version with VTK ?
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