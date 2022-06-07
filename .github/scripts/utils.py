from contextlib import contextmanager
import os
from pathlib import Path
import shutil
import zipfile


@contextmanager
def unzip_file(zip_path, unpack_path=None):
    # This will unzip the file while inside the context, and re-zip it
    # afterwards, which allows the caller to modify the contents

    if unpack_path is None:
        unpack_path = str(Path('__tmp_unzip_file_unpack').resolve())

    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(unpack_path)

    try:
        yield unpack_path
    finally:
        name = shutil.make_archive(zip_path, 'zip', unpack_path)
        shutil.rmtree(unpack_path)
        if name.endswith('.zip'):
            Path(name).rename(name[:-4])


@contextmanager
def change_dir(path):
    # Temporarily change the directory to the path. When the context ends,
    # the path will be changed back.

    origin = Path.cwd()
    try:
        os.chdir(path)
        yield
    finally:
        os.chdir(origin)
