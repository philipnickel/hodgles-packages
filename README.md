# Conda packages for hodgles

Recipes for the packages hodgles (high-order discontinuous Galerkin large
eddy simulations) needs that conda-forge does not provide. Everything else (hypre, METIS, zlib,
BLAS/LAPACK, HDF5, Open MPI) comes from conda-forge. The recipes are
rattler-build v1 recipes; `.github/workflows/packages.yml` builds them and
publishes them to the prefix.dev channel `philipnickel-hodgles`.

| Recipe | Package | What |
|---|---|---|
| `caliper/` | `caliper` 2.15.0 | LLNL Caliper, shared, MPI; upstream tarball plus upstream fix af80b77 (`caliper-iterator-cstdlib.patch`) |
| `gslib/` | `gslib` 1.0.9 | Nek5000 GSLIB, shared, MPI (MFEM's FindPointsGSLIB) |
| `reflect-cpp/` | `reflect-cpp` 0.26.2 | the option contract's serialization; its own README.md |
| `mfem/` | `mfem-hodgles` 4.10.0.post5 | stock upstream MFEM 4.10.0 (no patches): MPI, hypre, METIS, zlib, LAPACK, GSLIB, Caliper, parallel HDF5 |

Compared with the earlier `mfem-dgns` 4.10.0.post4 (built from a PyMFEM fork,
which vendored Caliper and a static GSLIB and hard-pinned Open MPI 5.0.10),
post5 differs: Caliper and GSLIB are their own packages
(GSLIB now shared), parallel HDF5 is on (`MFEM_USE_HDF5`, so
`mfem::ParaViewHDFDataCollection` writes one VTKHDF file for all ranks),
MPI is a variant whose run dependency comes from the MPI package's
run_exports, linux-64 has a generic and an x86-64-v3 build, and there is no
PyMFEM package. Like post4 it builds with
`-fno-visibility-inlines-hidden` (so a consumer's kernel specializations
reach libmfem's dispatch tables) and `-O3`. `run_exports` pins
`mfem-hodgles` exactly.

Recipe tests: package contents; caliper builds a consumer whose region shows
in a `runtime-report`; gslib checks `GSLIB_USE_MPI`; mfem checks the config
defines (MPI, GSLIB, CALIPER, HDF5 on; thread safety, MPFR, CEED off) and
`H5_HAVE_PARALLEL`, then builds `mfem/test-cmake/` with
`find_package(MFEM 4.10)` and runs on 2 ranks: the ParMesh smoke test, the
same with `CALI_CONFIG=runtime-report` (its `MFEM_PERF_SCOPE` region must
appear), and a ParaViewHDFDataCollection write of a ParMesh and an H1
GridFunction (two cycles; `hdf5-out/hdf5_smoke.vtkhdf` must be non-empty).

## Dependency order

```
caliper ─┐
gslib  ──┴─> mfem-hodgles ─┐
reflect-cpp ────────────┴─> hodgles-solver (built from the private hodgles repository)
```

Build caliper and gslib before mfem; reflect-cpp is independent.

## Variants

`variants.yaml` (linked into each MPI recipe directory, so rattler-build
finds it without `--variant-config`):

- `mpi`: `openmpi` only. Caliper, gslib and mfem take the MPI package in
  their host requirements; its run_exports (conda-forge's
  `openmpi >=5.0.x,<6`) set the run dependency. Build strings follow
  conda-forge: `mpi_openmpi_h<hash>_<number>`, and each package's
  run_exports carry the build-string constraint so downstream builds pick
  the same MPI (`caliper >=2.15.0,<2.16.0a0 mpi_openmpi_*`). hypre 3.x on
  conda-forge has no `mpi_` build string; the host MPI selects its build.
  HDF5 is `hdf5 * mpi_<mpi>_*`.
- `microarch_level` (mfem only): conda-forge's microarchitecture helper
  (knowledge base, "Microarchitecture-optimized builds"). On linux-64, mfem
  builds with `x86_64-microarch-level ==1` (conda-forge's generic
  `-march=nocona`) and `==3` (its activation appends `-march=x86-64-v3`:
  AVX2, FMA; Haswell and newer, every DTU hpc node). Their run_exports add
  `_x86_64-microarch-level >=N`, which only installs where `__archspec`
  allows; the v3 build has build number 200 (level 1: 0), so the solver
  prefers it wherever it can run. osx-arm64 builds once, with
  `-mcpu=apple-m1`.

### Enabling LUMI later (MPICH)

Add `- mpich` under `mpi:` in `variants.yaml`. Every MPI recipe then builds
an `mpi_mpich_*` variant next to the Open MPI one (conda-forge has MPICH
builds of hypre and HDF5), and an environment chooses by depending on
`mpich` or `openmpi` (on LUMI, conda-forge's `mpich=*=external_*` so Cray
MPICH is used at run time, inside the container). Recipe tests export only
`OMPI_*` variables, which MPICH ignores.

## Local builds

From the repository root, into an output directory that is also read as a
channel (absolute path, otherwise rattler-build looks it up on
anaconda.org):

```bash
rattler-build build --recipe caliper -c conda-forge -c "$PWD/out" --output-dir "$PWD/out"
```

```bash
rattler-build build --recipe gslib -c conda-forge -c "$PWD/out" --output-dir "$PWD/out"
```

```bash
rattler-build build --recipe reflect-cpp -c conda-forge -c "$PWD/out" --output-dir "$PWD/out"
```

```bash
rattler-build build --recipe mfem -c conda-forge -c "$PWD/out" --output-dir "$PWD/out"
```

`out/` is ignored by git. Each build runs its recipe tests. Add
`--skip-existing all -c https://prefix.dev/philipnickel/philipnickel-hodgles`
to skip builds whose hash is already published. linux-64
builds need a Linux host (CI); `--target-platform linux-64` from macOS
cross-compiles but cannot run the tests.

Bump `build.number` (the `build` context entry) for any change to a recipe
or patch; bump the version's `.postN` for `mfem-hodgles`. Published packages
are immutable.

## Publishing

`.github/workflows/packages.yml` runs on changes to the recipes (push,
pull request) and on manual dispatch. One job per platform (ubuntu-24.04
for linux-64, macos-14 for osx-arm64) builds caliper, gslib, reflect-cpp
and mfem in that order with `--skip-existing all` against the channel, runs
the recipe tests, and keeps the new packages as an artifact. A publish job
uploads them with `rattler-build upload prefix --generate-attestation` only
for a push to the default branch or a manual dispatch; pull requests build
and test only.

The upload uses prefix.dev trusted publishing (GitHub OIDC), so no API key
is stored. One-time setup on prefix.dev: in the channel's settings add a
trusted publisher for GitHub repository `philipnickel/hodgles-packages`, workflow
`packages.yml`. If trusted publishing is not wanted, create an API key, store it
as the repository secret `PREFIX_API_KEY` and pass
`PREFIX_API_KEY: ${{ secrets.PREFIX_API_KEY }}` to the upload step's
environment.

By hand (a local build, after review):

```bash
PREFIX_API_KEY=... rattler-build upload prefix --channel philipnickel/philipnickel-hodgles out/osx-arm64/mfem-hodgles-4.10.0.post5-*.conda
```

## Channels

- `philipnickel-hodgles`: these dependencies and the solver builds used for
  report results (releases).
- `philipnickel-hodgles-dev` (later): solver development builds
  (`hodgles-solver`, versioned by date and commit, published on demand before
  cluster campaigns), so development builds never mix with the release
  channel.

After a publish, the hodgles workspace reads `mfem-hodgles` and reflect-cpp
from `philipnickel-hodgles`.
