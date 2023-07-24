import pathlib
from setuptools import setup
from cmake_build_extension import BuildExtension, CMakeExtension

this_directory = pathlib.Path(__file__).parent
long_description = (this_directory / "README.md").read_text()
long_description_content_type="text/markdown"

setup(
    name="pylibvpx",
    version="1.0.1",
    author="Peter Würtz",
    author_email="pwuertz@gmail.com",
    url="https://github.com/pwuertz/libvpx-bindings",
    description="Python bindings for libvpx",
    long_description=long_description,
    long_description_content_type=long_description_content_type,
    ext_modules=[CMakeExtension(
        name="pylibvpx",
        source_dir=".",
        install_prefix=".",
    )],
    cmdclass=dict(build_ext=BuildExtension),
)
