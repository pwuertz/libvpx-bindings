# Python `libvpx` bindings
Python bindings for VPX encoding/decoding library

# Build with dependencies from lockfile
```shell
uv sync --no-install-project
uv build --wheel --no-build-isolation
```

# Develop with auto rebuild
```shell
uv sync
uv run pytest
```
