# reflect-cpp for the hodgles channel

reflect-cpp (option contract, `docs/architecture.md`) is not on conda-forge,
so it is built here as `reflect-cpp` 0.26.2 with two patches:

- `reflect-cpp-tagged-union-defaults.patch`: `rfl::json::to_schema` segfaulted
  for an `rfl::TaggedUnion` whose alternative has an `rfl::DefaultVal`
  member (it dereferenced a null alternative). With the patch the
  alternatives' defaults reach the schema. Not yet upstream.
- `reflect-cpp-config-version-name.patch`: upstream installs its version file
  as `reflectcpp-version.cmake`, which `find_package` never reads, so
  `find_package(reflectcpp 0.26)` failed.

JSON only, bundled yyjson/ctre/enchantum, shared library, C++20. The package
test builds `test-cmake/` (a versioned `find_package`, a tagged-union read and
its schema), as `mfem-hodgles` does.

## Build locally

From the repository root (see README.md for the dependency order and the
CI that publishes it):

```bash
pixi exec rattler-build build --recipe reflect-cpp -c conda-forge --output-dir out
```

Bump `build.number` for any change to the recipe or patches.

Only osx-arm64 is built. `cpp/pixi.toml` therefore lists `reflect-cpp` under
`[package.target.osx-arm64.host-dependencies]`; a linux-64 build (on a Linux
host, or `--target-platform linux-64` in CI) lets it
move to the plain host dependencies. Its `run_exports` (`>=0.26.2,<0.27`) put
it into the solver's run dependencies.

## Publish to the hodgles channel

Published packages are immutable. `.github/workflows/packages.yml` builds
this recipe for linux-64 and osx-arm64 (after caliper and gslib, before
mfem) and uploads it to `https://prefix.dev/philipnickel/hodgles`
on the default branch or a manual dispatch; `README.md` describes
the workflow, local builds and the prefix.dev setup. Once published, drop
the `file://` channel from `pyproject.toml`, move `reflect-cpp` from
`[package.target.osx-arm64.host-dependencies]` to the plain host
dependencies in `cpp/pixi.toml`, and run `pixi update reflect-cpp`.
