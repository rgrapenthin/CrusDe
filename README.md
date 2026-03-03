# CrusDe

## General Info / License

CrusDe is a plug-in based simulation framework to study effects due to changes
in the load resting on the Earth's surface. Such changes could for example
involve a melting glacier, annual snow load, a reservoir's alternating water
level, or lava flows. The focus in the simulation could be the response of the
Earth's crust in terms of stress changes, changes in strain rate, or simply
uplift or subsidence and the respective horizontal displacements of the crust
(over time).

CrusDe is distributed under the terms of the **GPLv2** license. CrusDe will
remain open source software. Whoever redistributes it must include the sources
for CrusDe and any extensions or changes made.

## Contents of Package

| Path | Description |
|------|-------------|
| `doc/` | Doxygen-generated documentation (run `make doc` to build) |
| `experiment-db/` | Experiment history database, viewed with `crusde -m` |
| `libs/` | Bundled third-party libraries (geographiclib-1.4) |
| `plugins/` | Plugin database; shared libraries are copied here on install |
| `src/` | C++ source code; run `make all` here to build |
| `testcases/` | Sample experiments illustrating the XML input format |

## Plugins

CrusDe ships the following standard plugins:

**Green's functions** (`green`)
- `elastic halfspace (pinel)` — elastic halfspace, Pinel et al. (2007)
- `thick plate (pinel)` — elastic layer over inviscid fluid
- `final relaxed (pinel)` — fully relaxed response (elastic + thick plate)
- `inverse thick plate (pinel)` — negative thick-plate response
- `elastic minus thickplate (pinel)` — transient component
- `alma` — viscoelastic spherical Earth via ALMA3 (Melini et al. 2022)

**Loads** (`load`)
- `disk load` — uniform cylindrical disk load
- `irregular load` — spatially irregular load read from an ASCII file

**Load histories** (`load_history`)
- `heaviside ramp` — step function
- `boxcar` — rectangular on/off window
- `boxcar_rate` — derivative of boxcar (impulses at edges)
- `dirac_impulse` — delta function
- `sinusoidal` / `sinusoidal_rate` — sinusoidal history and its rate
- `fourier_series` / `fourier_series_rate` — annual + semi-annual Fourier series

**Operators** (`operator`)
- `fast 2d convolution` — FFT-based 2-D spatial convolution (single load)
- `fast 3d convolution` — FFT-based 2-D spatial + 1-D temporal convolution

**Post-processors** (`postprocess`)
- `xy2r` — computes radial displacement r = √(x² + y²)
- `hori2vert-ratio` — ratio of horizontal to vertical displacement
- `factor` — scales all output fields by a constant

**Crustal decay** (`crustal_decay`)
- `exponential` / `exponential_rate` — exponential decay function and its rate

**Output** (`data_handler`)
- `netcdf writer` — writes results to a COARDS-compliant netCDF file
- `table writer` — writes results to a whitespace-separated ASCII table

## Quick Start

```bash
# Set environment variable (add to your shell profile)
export CRUSDE_HOME=/path/to/CrusDe

# Run an experiment
cd testcases/disk
crusde disk_elastic.xml
```

Results are written to the netCDF file specified in the experiment XML.
Use `ncview` or `ncdump` to inspect them.

## Contact

CrusDe was written by Ronni Grapenthin. The latest version, news, and
documentation can be found at:

- <https://www.grapenthin.org/crusde>
- <https://github.com/uafgeotools/CrusDe> *(check for current repository)*
