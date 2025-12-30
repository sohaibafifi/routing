# API Reference

## C++ API (Doxygen)

We generate C++ API docs with Doxygen and expose them in Sphinx via Breathe.

:::{note}
Run Doxygen first, then build Sphinx:

```bash
cd docs
# 1) Run doxygen
# doxygen Doxyfile

# 2) Build Sphinx
# make html
```
:::

```{toctree}
:maxdepth: 2

cpp-api
```

## Python API

The Python bindings live under `python/routing/` and are exposed via the
`routing` package.

```{toctree}
:maxdepth: 2

python-api
```
