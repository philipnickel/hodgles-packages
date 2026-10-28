// Minimal downstream consumer: find_package(MFEM), build a ParMesh, solve nothing.
#include "mfem.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
   mfem::Mpi::Init(argc, argv);
   mfem::Hypre::Init();
   const int rank = mfem::Mpi::WorldRank();
   MFEM_PERF_SCOPE("parmesh_smoke");

   mfem::Mesh serial = mfem::Mesh::MakeCartesian2D(
      8, 8, mfem::Element::QUADRILATERAL);
   mfem::ParMesh mesh(MPI_COMM_WORLD, serial);
   serial.Clear();

   mfem::H1_FECollection fec(2, mesh.Dimension());
   mfem::ParFiniteElementSpace fes(&mesh, &fec);
   const HYPRE_BigInt global_dofs = fes.GlobalTrueVSize();
   const long long global_elements = mesh.GetGlobalNE();

   if (global_elements != 64 || global_dofs != 289)
   {
      if (rank == 0)
      {
         std::cerr << "unexpected sizes: " << global_elements << " elements, "
                   << global_dofs << " dofs" << std::endl;
      }
      return 1;
   }
   if (rank == 0)
   {
      std::cout << "MFEM " << MFEM_VERSION_STRING << " ParMesh on "
                << mfem::Mpi::WorldSize() << " ranks: " << global_elements
                << " elements, " << global_dofs << " H1(p=2) dofs" << std::endl;
   }
   return 0;
}
