from setuptools import setup
from cmake_build_extension import BuildExtension, CMakeExtension

setup(
    name="pylibvpx",
    description="Python bindings for libvpx",
    url="https://github.com/pwuertz/libvpx-bindings",
    version="1.0",
    author="Peter Würtz",
    author_email="pwuertz@gmail.com",
    ext_modules=[CMakeExtension(
        name="pylibvpx",
        source_dir=".",
        install_prefix=".",
    )],
    cmdclass=dict(build_ext=BuildExtension),
)
