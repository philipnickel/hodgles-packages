#!/usr/bin/env bash
# Build stock MFEM 4.10 (upstream tarball, no source patches) into $PREFIX
# against conda-forge's hypre, METIS, zlib, BLAS/LAPACK and parallel HDF5 and
# this directory's sibling packages caliper and gslib. Feature set:
#   mfem@4.10.0 +mpi +metis +zlib +lapack +gslib +caliper +hdf5 ~mpfr
#               ~threadsafe ~libceed ~openmp cxxstd=17 (hypre, CPU only)
set -euxo pipefail

# ---- Compiler flags --------------------------------------------------------
# Conda-forge's CXXFLAGS carry -O2, which comes after CMake's Release -O3 in
# config.mk; append -O3 so the partial-assembly kernels are optimized.
# The CPU baseline: on linux-64 the x86_64-microarch-level build dependency
# appends -march=x86-64-v3 for the v3 variant (and nothing for the generic
# one, which keeps conda-forge's -march=nocona); on osx-arm64 Apple M1.
# Conda-forge's CXXFLAGS also carry -fvisibility-inlines-hidden, which hides
# the function-local static of every inline Kernels::Get() (the PA and
# quadrature-interpolator dispatch tables, fem/kernel_dispatch.hpp). A
# consumer's AddSpecialization calls then fill a private copy of the table
# that libmfem never reads. Export inline functions so the tables coalesce.
export CXXFLAGS="${CXXFLAGS:-} -fno-visibility-inlines-hidden -O3"
if [[ "${target_platform}" == osx-arm64 ]]; then
  export CXXFLAGS="${CXXFLAGS} -mcpu=apple-m1"
fi

# ---- MFEM ------------------------------------------------------------------
cmake -S . -B build -G Ninja \
  ${CMAKE_ARGS} \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_LIBDIR=lib \
  -DCMAKE_CXX_STANDARD=17 \
  -DBUILD_SHARED_LIBS=ON \
  -DMPI_C_COMPILER="$PREFIX/bin/mpicc" \
  -DMPI_CXX_COMPILER="$PREFIX/bin/mpicxx" \
  -DMFEM_USE_MPI=ON \
  -DHYPRE_DIR="$PREFIX" \
  -DMFEM_USE_METIS=ON \
  -DMFEM_USE_METIS_5=ON \
  -DMETIS_DIR="$PREFIX" \
  -DMFEM_USE_ZLIB=ON \
  -DZLIB_DIR="$PREFIX" \
  -DMFEM_USE_LAPACK=ON \
  -DBLAS_LIBRARIES="$PREFIX/lib/libblas${SHLIB_EXT}" \
  -DLAPACK_LIBRARIES="$PREFIX/lib/liblapack${SHLIB_EXT}" \
  -DMFEM_USE_GSLIB=ON \
  -DGSLIB_DIR="$PREFIX" \
  -DMFEM_USE_CALIPER=ON \
  -DCALIPER_DIR="$PREFIX" \
  -DMFEM_USE_HDF5=ON \
  -DHDF5_DIR="$PREFIX" \
  -DMFEM_USE_MPFR=OFF \
  -DMFEM_THREAD_SAFE=OFF \
  -DMFEM_USE_OPENMP=OFF \
  -DMFEM_USE_CEED=OFF \
  -DMFEM_USE_CUDA=OFF \
  -DMFEM_USE_EXCEPTIONS=OFF \
  -DMFEM_ENABLE_TESTING=OFF \
  -DMFEM_ENABLE_EXAMPLES=OFF \
  -DMFEM_ENABLE_MINIAPPS=OFF

cmake --build build --parallel "${CPU_COUNT}"
cmake --install build

# The build-environment compiler path recorded by MFEMConfig.cmake does not
# exist in a consumer environment; point it at the packaged MPI wrapper.
sed -i.bak -E \
  "s|^set\\(MFEM_CXX_COMPILER \".*\"\\)|set(MFEM_CXX_COMPILER \"$PREFIX/bin/mpicxx\")|" \
  "$PREFIX/lib/cmake/mfem/MFEMConfig.cmake"
rm -f "$PREFIX/lib/cmake/mfem/MFEMConfig.cmake.bak"
