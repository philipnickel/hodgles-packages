// Parallel VTKHDF output: every rank writes its part of a ParMesh and an H1
// GridFunction into one shared .vtkhdf file through parallel HDF5 (MPI-IO).
// MFEM aborts here if its HDF5 lacks parallel support (MFEM_PARALLEL_HDF5).
#include "mfem.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
   mfem::Mpi::Init(argc, argv);
   mfem::Hypre::Init();

   mfem::Mesh serial = mfem::Mesh::MakeCartesian2D(
      8, 8, mfem::Element::QUADRILATERAL);
   mfem::ParMesh mesh(MPI_COMM_WORLD, serial);
   serial.Clear();

   mfem::H1_FECollection fec(2, mesh.Dimension());
   mfem::ParFiniteElementSpace fes(&mesh, &fec);
   mfem::ParGridFunction u(&fes);
   mfem::FunctionCoefficient xy([](const mfem::Vector &x) { return x(0) + x(1); });
   u.ProjectCoefficient(xy);

   mfem::ParaViewHDFDataCollection dc("hdf5_smoke", &mesh);
   dc.SetPrefixPath("hdf5-out");
   dc.RegisterField("u", &u);
   for (int cycle = 0; cycle < 2; ++cycle)
   {
      dc.SetCycle(cycle);
      dc.SetTime(0.5 * cycle);
      dc.Save();
   }

   if (mfem::Mpi::Root())
   {
      std::cout << "wrote hdf5-out/hdf5_smoke.vtkhdf from "
                << mfem::Mpi::WorldSize() << " ranks" << std::endl;
   }
   return 0;
}
