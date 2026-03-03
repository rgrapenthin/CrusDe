# Changelog

## 2026-03-03 — Build modernization and ALMA3 integration

### Build system
- Ported build to Ubuntu 22.04 / Qt5; dropped all Qt4 (Trolltech) dependencies
- `src/Makefile`: replaced hardcoded Qt4/Trolltech paths with `pkg-config`
  queries for `Qt5Widgets` and `Qt5Core`; fixed `MOC` binary lookup via
  `pkg-config --variable=host_bins Qt5Core`; fixed `xerces-c` include/lib
  paths via `pkg-config`; updated GeographicLib path from deleted symlink
  `libs/geographiclib` to `libs/geographiclib-1.4`; excluded auto-generated
  `moc_*.cpp` files from `SOURCE` list to prevent double-linking of Qt MOC
  objects
- `src/plugin_src/operator/Makefile`: removed stale `/usr/local/fftw/api`
  include path (`fftw3.h` is now found via system include path)

### Source compatibility
- `src/Qt_TreeModel.cpp`: replaced `#include <QtGui>` with `#include
  <QtWidgets>` for Qt5; replaced removed `reset()` call with
  `beginResetModel()` / `endResetModel()` pair
- `src/DatabaseManager.cpp`: fixed `QApplication` constructor — Qt5 requires
  `int&` for `argc`, changed `QApplication app(0, 0)` to
  `QApplication app(argc, nullptr)` with `static int argc = 0`
- `src/defs.h`: deleted stale auto-generated dependency file containing old
  macOS include paths (regenerated correctly by `make`)
- `libs/geographiclib-1.4/include/GeographicLib/Geoid.hpp`: replaced
  `std::ios::streamoff` with `std::streamoff` for C++11 compatibility

### ALMA3 Green's function plugin (`src/plugin_src/green/alma.c`)
- Rewrote plugin to interface with
  [ALMA3](https://github.com/danielemelini/ALMA3) (Melini et al. 2022)
  instead of the earlier ALMA2 API
- Removed parameters `kv`, `lth`, `mode`, `rheol` (now encoded in the ALMA3
  rheological model file)
- Added parameters: `ng` (Gaver/Salzer order), `nla` (number of layers),
  `p` (time subdivisions), `sd` (significant digits), `time_min`, `time_max`
- `init()`: writes a temporary ALMA3 config file to `/tmp`, invokes
  `$ALMA/alma.exe`, reads the resulting Love number files (`h`, `l`, `k`) in
  `ln_vs_n` format, then removes all temp files
- `read_alma_output()`: rewritten to parse ALMA3's space-separated output
  (rows = harmonic degree, columns = time steps)
- `get_value_at()`: implemented Farrell (1972) loading Green's function via
  Legendre polynomial series — `G_z = -(a/M_E) Σ h_n P_n(cos θ)`,
  `G_h = -(a/M_E) Σ l_n dP_n/dθ` — using three-term recurrence with Taylor
  fallback near the origin; time index taken from `crusde_model_step()`
- `$ALMA` environment variable must point to the directory containing
  `alma.exe`; `file` parameter gives the ALMA3 rheological model file
  (absolute path or relative to `$ALMA`)

### Testcases
- `testcases/alma/disk_elastic.xml`: updated for ALMA3 parameter set
  (removed old ALMA2 params; set `deg_max=100`, `nla=3`, `p=0` for
  instantaneous elastic response; `file=MODELS/simple-Earth.dat`)

---

## 0.3.0

- Fixed bug in `netcdf writer` that caused incorrect output on x86_64
  architectures (type mismatch between written and declared variable types)
- Fixed bug in `pinel_hs_elastic.c` where grid size was not taken into account
  when calculating radial distances
- Added `geographiclib` to `libs/` for future geodesic calculation support
- Implemented and tested `conv.c` (2-D spatial-domain convolution) for single
  disk case
- Fixed sign bug in Pinel Green's functions: absolute values of x/y indices
  were needed in the spatial convolution path
- Added `fourier_series` load history plugin for annual + semi-annual load
  history superposition; added corresponding testcase in `testcases/disk/`
- Increased maximum number of simultaneous load functions to 1000; added error
  message on overflow (previously a silent segfault)
- Fixed typo in `crusde_info` call
- Reorganized directory structure
- Completed API documentation in `crusde_api.cpp`; rebuilt Doxygen docs
- Added `debug` make target: sets `-DDEBUG` for verbose `crusde_debug` output;
  `all` remains the release target
- Added `crusde -p install <plugin.so>` command-line interface for headless
  plugin registration (used by `install_plugins.sh`)
- `install_plugins.sh` now verifies prerequisites and aborts on error
- Added optional parameter support: `crusde_register_optional_param*` API
  functions accept a default value; `InputHandler` skips optional params not
  present in the XML
- Added `crusde_info`, `crusde_warning`, `crusde_error`, `crusde_debug` output
  functions usable from both plugins and core code
- Added `-q`/`-Q` quiet mode: suppresses info messages; errors, warnings, and
  debug output are unaffected
- `irregular load` now skips comment and blank lines; aborts with a clear error
  message on parse failure
- Fixed `crusde -p install` in `InputHandler` (was broken after the
  XML-existence check was added)
- Changed coordinate unit label from `degrees_lat/lon` to `meters_lat/lon`

---

## 0.2.0 → 0.1.x

- Removed all command-line model building; experiment definition is now
  exclusively via XML files (`InputHandler`)
- Replaced `<load_function>` tag with `<load>` wrapping both load and load
  history
- Introduced `LoadFunction` class to support multiple independent loads with
  separate histories; `setHistoryFunction` is now mandatory in load plugins
