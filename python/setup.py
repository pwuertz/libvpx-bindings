from setuptools import find_packages, setup
from cmake_build_extension import BuildExtension, CMakeExtension

setup(
    name="pylibvpx",
    version="0.1",
    author="Peter Würtz",
    description="Python bindings for libvpx",
    ext_modules=[CMakeExtension(
        name="pylibvpx",
        source_dir=".",
        install_prefix=".",
    )],
    cmdclass=dict(build_ext=BuildExtension),
)
