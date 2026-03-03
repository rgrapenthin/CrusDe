# Installing CrusDe

## System Requirements

CrusDe builds on Linux with a standard C++/C toolchain. The instructions below
target **Ubuntu 22.04 LTS** (or any similar modern Debian/Ubuntu system) but
should adapt easily to other distributions.

## 1. Install Dependencies

Install all required development packages with:

```bash
sudo apt-get install -y \
    build-essential \
    libfftw3-dev \
    libgsl-dev \
    libnetcdf-dev \
    libxerces-c-dev \
    qtbase5-dev \
    gfortran
```

| Package | Used by |
|---------|---------|
| `libfftw3-dev` | `fast_conv` and `fast_conv_time_space` operator plugins |
| `libgsl-dev` | Pinel Green's function plugins |
| `libnetcdf-dev` | `netcdf writer` output plugin |
| `libxerces-c-dev` | XML parsing throughout CrusDe |
| `qtbase5-dev` | Plugin manager GUI |
| `gfortran` | ALMA3 (only needed for the `alma` Green's function plugin) |

## 2. Set Up the Source Directory

Place the CrusDe source anywhere you like and set `CRUSDE_HOME`:

```bash
export CRUSDE_HOME=/path/to/CrusDe   # e.g. /home/you/CrusDe
# Add the above line to ~/.bashrc or ~/.profile to make it permanent
```

## 3. Build the Bundled GeographicLib

CrusDe bundles GeographicLib 1.4 in `libs/geographiclib-1.4/`. Build it once:

```bash
cd $CRUSDE_HOME/libs/geographiclib-1.4
make -f Makefile.mk
```

This produces `libs/geographiclib-1.4/src/libGeographic.a`.

## 4. (Optional) Install ALMA3 for Viscoelastic Green's Functions

The `alma` Green's function plugin interfaces with
[ALMA3](https://github.com/danielemelini/ALMA3) (Melini et al. 2022), a
Fortran program for computing planetary Love numbers. Skip this step if you
do not need viscoelastic/spherical-Earth deformation.

```bash
# Clone and build ALMA3
git clone https://github.com/danielemelini/ALMA3 ~/work/CODE/ALMA
cd ~/work/CODE/ALMA
make        # requires gfortran and the FMLIB multi-precision library (bundled)

# Set the environment variable (add to ~/.bashrc or ~/.profile)
export ALMA=/path/to/ALMA   # directory containing alma.exe
```

ALMA3 rheological model files live in `$ALMA/MODELS/`. The experiment XML
refers to them via the `file` parameter (relative to `$ALMA`, or as an
absolute path).

## 5. Build CrusDe

```bash
cd $CRUSDE_HOME/src
make all
```

The binary `crusde` is produced in `$CRUSDE_HOME/` and also copied to
`/usr/local/bin`. If you do not have write access to `/usr/local/bin`, change
the `BIN` variable in `src/Makefile` before building:

```makefile
BIN := $(HOME)/bin   # or any directory on your PATH
```

To build a debug version (verbose output, `-DDEBUG`):

```bash
make debug
```

## 6. Install the Plugins

The `install_plugins.sh` script registers all standard plugins with CrusDe's
plugin database:

```bash
cd $CRUSDE_HOME/src
bash install_plugins.sh
```

This calls `crusde -p install <plugin.so>` for each plugin in the correct
dependency order. If the script fails, you can register plugins individually:

```bash
crusde -p install $CRUSDE_HOME/plugins/green/pinel_hs_elastic.so
crusde -p install $CRUSDE_HOME/plugins/green/pinel_hs_thickplate.so
# ... etc.
```

Or use the GUI plugin manager interactively:

```bash
crusde -p
```

The recommended install order (to satisfy inter-plugin dependencies):

1. **crustal\_decay**: `exponential.so`, `exponential_rate.so`
2. **data\_handler**: `netcdf.so`, `table.so`
3. **green**: `pinel_hs_elastic.so`, `pinel_hs_thickplate.so`,
   `pinel_hs_final_relaxed.so`, `pinel_inverse_thickplate.so`,
   `pinel_hs_elastic_minus_thickplate.so`, `alma.so`
4. **load**: `disk_load.so`, `irregular_load.so`
5. **load\_history**: all (any order)
6. **operator**: `fast_conv.so` first, then `fast_conv_time_space.so`
7. **postprocess**: all (any order)

## 7. Verify the Installation

Run the included testcases. Each must be run from its own directory so that
relative paths in the XML files resolve correctly:

```bash
cd $CRUSDE_HOME/testcases/disk
crusde disk_elastic.xml      # should print "Terminating normally! Bye."

cd $CRUSDE_HOME/testcases/icelandic_rhythmics
crusde elastic.xml

cd $CRUSDE_HOME/testcases/alma
ALMA=/path/to/ALMA crusde disk_elastic.xml   # requires ALMA3
```

## License

CrusDe is free software; you can redistribute it and/or modify it under the
terms of the **GNU General Public License version 2** as published by the Free
Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

---

Ronni Grapenthin
<ronni@grapenthin.org>
