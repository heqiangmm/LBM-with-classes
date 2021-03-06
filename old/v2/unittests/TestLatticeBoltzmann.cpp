#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "Lattice.hpp"
#include "UnitTest++.h"

SUITE(FunctionalityAndExceptionTests)
{
static const double zero_tol = 1e-20;
static const double loose_tol = 1e-5;
enum DiscreteDirections {
  E = 1,
  N,
  W,
  S,
  NE,
  NW,
  SW,
  SE,
};
static const std::size_t g_num_dimensions = 2;
static const std::size_t g_num_discrete_velocities = 9;
static const std::size_t g_num_rows = 5;
static const std::size_t g_num_cols = 4;
static const double g_dx = 0.0316;
static const double g_dt = 0.001;
static const double g_t_total = 1.0;
static const double g_diffusion_coefficient = 0.2;
static const double g_kinematic_viscosity = 0.2;
static const std::vector<double> g_u0 = {123, 321};
static const std::vector<double> g_u0_zero = {0, 0};
static const std::vector<std::vector<unsigned>> g_src_pos_f = {{0, 0}};
static const std::vector<std::vector<unsigned>> g_src_pos_g = {{0, 0}};
static const std::vector<std::vector<double>> g_src_strength_f = {{1, 1}};
static const std::vector<double> g_src_strength_g = {1};
static const std::vector<std::vector<unsigned>> g_obstacles_pos = {{0, 0}};
static const bool g_is_cd = true;
static const bool g_is_ns = true;
static const bool g_is_instant = true;
static const bool g_has_obstacles = true;
static const bool g_is_not_cd = false;
static const bool g_is_not_ns = false;
static const bool g_is_not_instant = false;
static const bool g_obstacles = true;
static const bool g_no_obstacles = false;
static const double g_density_g = 2.0;
static const double g_density_f = 2.0;
static const Lattice g_lattice(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, g_u0, g_src_pos_f,
      g_src_pos_g, g_src_strength_f, g_src_strength_g, g_obstacles_pos, g_is_cd,
      g_is_ns, g_is_not_instant, g_no_obstacles);

// Functionality Tests
TEST(DefaultLattice)
{
  Lattice lattice;
  CHECK_EQUAL(0u, lattice.GetNumberOfDimensions());
  CHECK_EQUAL(0u, lattice.GetNumberOfDiscreteVelocities());
  CHECK_EQUAL(0u, lattice.GetNumberOfRows());
  CHECK_EQUAL(0u, lattice.GetNumberOfColumns());
}

TEST(NormalLatticeBeforeInit)
{
  Lattice lattice(g_lattice);
  std::size_t ny = lattice.GetNumberOfRows();
  std::size_t nx = lattice.GetNumberOfColumns();
  std::size_t nd = lattice.GetNumberOfDimensions();
  std::size_t nc = lattice.GetNumberOfDiscreteVelocities();
  std::size_t lattice_size = ny * nx;
  CHECK_EQUAL(2u, nd);
  CHECK_EQUAL(9u, nc);
  CHECK_EQUAL(5u, ny);
  CHECK_EQUAL(4u, nx);
  for (auto n = 0u; n < lattice_size; ++n) {
    CHECK_EQUAL(nc, lattice.f[n].size());
    CHECK_EQUAL(nc, lattice.g[n].size());
    CHECK_EQUAL(nc, lattice.f_eq[n].size());
    CHECK_EQUAL(nc, lattice.g_eq[n].size());
    CHECK_EQUAL(nd, lattice.src_f[n].size());
    CHECK_EQUAL(nd, lattice.u[n].size());
    for (auto i = 0u; i < nc; ++i) {
      CHECK_CLOSE(0.0, lattice.f[n][i], zero_tol);
      CHECK_CLOSE(0.0, lattice.g[n][i], zero_tol);
      CHECK_CLOSE(0.0, lattice.f_eq[n][i], zero_tol);
      CHECK_CLOSE(0.0, lattice.g_eq[n][i], zero_tol);
    }  // i
    for (auto d = 0u; d < nd; ++d) {
      CHECK_CLOSE(0.0, lattice.src_f[n][d], zero_tol);
      CHECK_CLOSE(0.0, lattice.u[n][d], zero_tol);
    }  // d
    CHECK_CLOSE(0.0, lattice.src_g[n], zero_tol);
    CHECK_CLOSE(0.0, lattice.rho_f[n], zero_tol);
    CHECK_CLOSE(0.0, lattice.rho_g[n], zero_tol);
  }  // n
}

TEST(Init1DLattices)
{
  Lattice lattice(g_lattice);
  const double initial_rho = 1.0;
  lattice.Init(lattice.rho_f, initial_rho);
  lattice.Init(lattice.rho_g, initial_rho);
  for (auto node : lattice.rho_f) {
    CHECK_CLOSE(initial_rho, node, zero_tol);
  }
  for (auto node : lattice.rho_g) {
    CHECK_CLOSE(initial_rho, node, zero_tol);
  }
}

TEST(Init2DLatticesWithValues)
{
  Lattice lattice(g_lattice);
  std::vector<double> initial_u = {1.0, 2.0};
  lattice.Init(lattice.u, initial_u);
  for (auto node : lattice.u) {
    CHECK_CLOSE(initial_u[0], node[0], zero_tol);
    CHECK_CLOSE(initial_u[1], node[1], zero_tol);
  }
}

TEST(Init2DLatticesWithLattice)
{
  Lattice lattice(g_lattice);
  auto ny = lattice.GetNumberOfRows();
  auto nx = lattice.GetNumberOfColumns();
  std::vector<double> initial_node = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector<std::vector<double>> initial_g(nx * ny, initial_node);
  lattice.Init(lattice.g, initial_g);
  for (auto node : lattice.g) {
    int index = 0;
    for (auto i : node) CHECK_CLOSE(initial_node[index++], i, zero_tol);
  }  // lat
}

TEST(CalculateEquilibrium)
{
  std::vector<double> u0 = {1., 2.};
  Lattice lattice(g_lattice);
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  std::vector<double> ans = {0.882213,
      0.242652, 0.266753, 0.200458, 0.182365,
      0.073214, 0.060663, 0.041569, 0.050114};
  lattice.Init(lattice.u, u0);
  lattice.Init(lattice.rho_g, g_density_g);
  lattice.ComputeEq(lattice.g_eq, lattice.rho_g);

  for (auto lat_eq : lattice.g_eq) {
    for (auto i = 0u; i < nc; ++i) {
      CHECK_CLOSE(ans[i], lat_eq[i], loose_tol);
    }  // i
  }  // lat_eq
}

TEST(InitFWithValueFromFEqNode)
{
  std::vector<double> u0 = {123., 321.};
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  lattice.Init(lattice.u, u0);
  lattice.Init(lattice.rho_g, g_density_g);
  lattice.ComputeEq(lattice.g_eq, lattice.rho_g);
  lattice.Init(lattice.g, lattice.g_eq);
  for (auto n = 0u; n < nx * ny; ++n) {
    for (auto i = 0u; i < nc; ++i) {
      CHECK_CLOSE(lattice.g_eq[n][i], lattice.g[n][i], zero_tol);
    }  // i
  }  // n
}

TEST(InitSingleSource)
{
  unsigned x_pos = 1;
  unsigned y_pos = 2;
  std::vector<std::vector<unsigned>> src_position = {{x_pos, y_pos}};
  std::vector<double> src_strength = {2};
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  lattice.Init(lattice.src_g, src_position, src_strength);
  for (auto n = 0u; n < nx * ny; ++n) {
    if (n == y_pos * nx + x_pos) {
      CHECK_CLOSE(src_strength[0], lattice.src_g[n], zero_tol);
    }
    else {
      CHECK_CLOSE(0.0, lattice.src_g[n], zero_tol);
    }
  }  // n
}

TEST(InitMultipleSource)
{
  unsigned x_pos = 2;
  unsigned y_pos = 1;
  std::vector<std::vector<unsigned>> src_position = {{x_pos, y_pos},
      {x_pos + 1, y_pos + 2}};
  std::vector<double> src_strength = {2, 3.4};
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  lattice.Init(lattice.src_g, src_position, src_strength);
  for (auto n = 0u; n < nx * ny; ++n) {
    if (n == y_pos * nx + x_pos) {
      CHECK_CLOSE(src_strength[0], lattice.src_g[n], zero_tol);
    }
    else if (n == (y_pos + 2) * nx + x_pos + 1) {
      CHECK_CLOSE(src_strength[1], lattice.src_g[n], zero_tol);
    }
    else {
      CHECK_CLOSE(0.0, lattice.src_g[n], zero_tol);
    }
  }  // n
}

TEST(InitSingle2DSource)
{
  unsigned x_pos = 1;
  unsigned y_pos = 2;
  std::vector<std::vector<unsigned>> src_position = {{x_pos, y_pos}};
  std::vector<std::vector<double>> src_strength = {{2, 3}};
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  lattice.Init(lattice.src_f, src_position, src_strength);
  for (auto n = 0u; n < nx * ny; ++n) {
    if (n == y_pos * nx + x_pos) {
      CHECK_CLOSE(src_strength[0][0], lattice.src_f[n][0], zero_tol);
      CHECK_CLOSE(src_strength[0][1], lattice.src_f[n][1], zero_tol);
    }
    else {
      CHECK_CLOSE(0.0, lattice.src_f[n][0], zero_tol);
      CHECK_CLOSE(0.0, lattice.src_f[n][1], zero_tol);
    }
  }  // n
}

TEST(InitMultiple2DSource)
{
  unsigned x_pos = 1;
  unsigned y_pos = 1;
  std::vector<std::vector<unsigned>> src_position = {{x_pos, y_pos},
      {x_pos + 2, y_pos + 1}};
  std::vector<std::vector<double>> src_strength = {{2, 3}, {2.3, 4.4}};
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  lattice.Init(lattice.src_f, src_position, src_strength);
  for (auto n = 0u; n < nx * ny; ++n) {
    if (n == y_pos * nx + x_pos) {
      CHECK_CLOSE(src_strength[0][0], lattice.src_f[n][0], zero_tol);
      CHECK_CLOSE(src_strength[0][1], lattice.src_f[n][1], zero_tol);
    }
    else if (n == (y_pos + 1) * nx + x_pos + 2) {
      CHECK_CLOSE(src_strength[1][0], lattice.src_f[n][0], zero_tol);
      CHECK_CLOSE(src_strength[1][1], lattice.src_f[n][1], zero_tol);
    }
    else {
      CHECK_CLOSE(0.0, lattice.src_f[n][0], zero_tol);
      CHECK_CLOSE(0.0, lattice.src_f[n][1], zero_tol);
    }
  }  // n
}

TEST(BoundaryConditionPeriodic)
{
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  std::vector<double> nums = {0, 1, 0, 2, 4, 5, 6, 7, 8};
  lattice.Init(lattice.g, nums);
  lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
  for (auto y = 0u; y < ny; ++y) {
    auto n = y * (nx);
    CHECK_CLOSE(lattice.g[n + nx - 1][E], lattice.boundary_g[y][E], zero_tol);
    CHECK_CLOSE(lattice.g[n + nx - 1][NE], lattice.boundary_g[y][NE],
        zero_tol);
    CHECK_CLOSE(lattice.g[n + nx - 1][SE], lattice.boundary_g[y][SE],
        zero_tol);
    CHECK_CLOSE(lattice.g[n][W], lattice.boundary_g[y + ny][W], zero_tol);
    CHECK_CLOSE(lattice.g[n][NW], lattice.boundary_g[y + ny][NW], zero_tol);
    CHECK_CLOSE(lattice.g[n][SW], lattice.boundary_g[y + ny][SW], zero_tol);
  }  // y
}

TEST(BoundaryConditionBounceBack)
{
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  std::vector<double> nums = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  double value = 0;
  for (auto &node : lattice.g) node = std::vector<double>(nc, value++);
  lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
  auto top = 2 * ny;
  auto bottom = 2 * ny + nx;
  for (auto x = 0u; x < nx; ++x) {
    auto n = (ny - 1) * (nx);
    CHECK_CLOSE(lattice.g[x + n][N], lattice.boundary_g[top + x][S],
        zero_tol);
    CHECK_CLOSE(lattice.g[x][S], lattice.boundary_g[bottom + x][N],
        zero_tol);
    if (x == 0) {
      CHECK_CLOSE(lattice.g[n + nx - 1][NE], lattice.boundary_g[top + x][SW],
          zero_tol);
      CHECK_CLOSE(lattice.g[nx - 1][SE], lattice.boundary_g[bottom + x][NW],
          zero_tol);
    }
    else {
      CHECK_CLOSE(lattice.g[x + n - 1][NE], lattice.boundary_g[top + x][SW],
          zero_tol);
      CHECK_CLOSE(lattice.g[x - 1][SE], lattice.boundary_g[bottom + x][NW],
          zero_tol);
    }
    if (x == nx - 1) {
      CHECK_CLOSE(lattice.g[n][NW], lattice.boundary_g[top + x][SE],
          zero_tol);
      CHECK_CLOSE(lattice.g[0][SW], lattice.boundary_g[bottom + x][NE],
          zero_tol);
    }
    else {
      CHECK_CLOSE(lattice.g[x + n + 1][NW], lattice.boundary_g[top + x][SE],
          zero_tol);
      CHECK_CLOSE(lattice.g[x + 1][SW], lattice.boundary_g[bottom + x][NE],
          zero_tol);
    }
  }
}

TEST(BoundaryConditionCorner)
{
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  std::vector<double> nums = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  lattice.Init(lattice.g, nums);
  lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
  auto corner = 2 * nx + 2 * ny;
  CHECK_CLOSE(lattice.g[0][SW], lattice.boundary_g[corner][NE], zero_tol);
  CHECK_CLOSE(lattice.g[nx - 1][SE], lattice.boundary_g[corner + 1][NW],
      zero_tol);
  CHECK_CLOSE(lattice.g[(ny - 1) * nx][NW],
      lattice.boundary_g[corner + 2][SE], zero_tol);
  CHECK_CLOSE(lattice.g[ny * nx - 1][NE], lattice.boundary_g[corner + 3][SW],
      zero_tol);
}

TEST(StreamHorizontal)
{
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  std::vector<double> first_three = {1, 2, 1, 0, 1, 2, 0, 0, 2};
  std::vector<double> second_three = {4, 5, 4, 3, 4, 5, 3, 3, 5};
  std::vector<std::vector<double>> nodes = {first_three, second_three};
  std::vector<double> first_result = {1, 5, 1, 3, 1, 5, 3, 3, 5};
  std::vector<double> second_result = {4, 2, 4, 0, 4, 2, 0, 0, 2};
  int counter = 0;
  for (auto &node : lattice.g) node = nodes[counter++ % 2];
  lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
  for (auto n = 0u; n < lattice.boundary_g.size(); ++n) {
    if (n < ny) {
      lattice.boundary_g[n] = nodes[1];
    }
    else if (n < 2 * ny) {
      lattice.boundary_g[n] = nodes[(ny + 1) % 2];
    }
    else if (n < 2 * ny + nx) {
      lattice.boundary_g[n] = nodes[n % 2];
    }
    else if (n < 2 * ny + 2 * nx) {
      lattice.boundary_g[n] = nodes[n % 2];
    }
    else {
      lattice.boundary_g[n] = nodes[(n + 1) % 2];
    }
  }  // n
  lattice.g = lattice.Stream(lattice.g, lattice.boundary_g);
  for (auto node : lattice.g) {
    for (auto i = 0u; i < nc; ++i) {
      if ((node[0] - 1) < zero_tol) {
      CHECK_CLOSE(first_result[i], node[i], zero_tol);
      }
      else {
        CHECK_CLOSE(second_result[i], node[i], zero_tol);
      }
    }  // i
  }  // lat
}

TEST(StreamVertical)
{
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  std::vector<double> first_three = {1, 1, 0, 1, 2, 0, 0, 2, 2};
  std::vector<double> second_three = {4, 4, 3, 4, 5, 3, 3, 5, 5};
  std::vector<std::vector<double>> nodes = {first_three, second_three};
  std::vector<double> first_result = {1, 1, 3, 1, 5, 3, 3, 5, 5};
  std::vector<double> second_result = {4, 4, 0, 4, 2, 0, 0, 2, 2};
  int counter = 0;
  for (auto &node : lattice.g) node = nodes[(counter++ / nx) % 2];
  lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
  for (auto n = 0u; n < lattice.boundary_g.size(); ++n) {
    if (n < ny) {
      lattice.boundary_g[n] = nodes[n % 2];
    }
    else if (n < 2 * ny) {
      lattice.boundary_g[n] = nodes[(n + 1) % 2];
    }
    else if (n < 2 * ny + 2 * nx) {
      lattice.boundary_g[n] = nodes[1];
    }
    else {
      lattice.boundary_g[n] = nodes[(n % 2 + (n + 1) % 2) % 2];
    }
  }  // n
  lattice.g = lattice.Stream(lattice.g, lattice.boundary_g);
  for (auto node : lattice.g) {
    for (auto i = 0u; i < nc; ++i) {
      if ((node[0] - 1) < zero_tol) {
      CHECK_CLOSE(first_result[i], node[i], zero_tol);
      }
      else {
        CHECK_CLOSE(second_result[i], node[i], zero_tol);
      }
    }  // i
  }  // lat
}

TEST(StreamDiagonalNESW)
{
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  std::vector<double> ones(9, 1);
  std::vector<double> twos(9, 2);
  std::vector<double> threes(9, 3);
  std::vector<std::vector<double>> nodes = {ones, twos, threes};
  std::vector<double> result = {1, 2, 3};
  for (auto y = 0u; y < ny; ++y) {
    for (auto x = 0u; x < nx; ++x) {
      auto n = y * nx + x;
      lattice.g[n] = nodes[(x % 3 + y % 3) % 3];
    }  // x
  }  // y
  lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
  for (auto n = 0u; n < lattice.boundary_g.size(); ++n) {
    if (n < ny) {
      lattice.boundary_g[n] = nodes[(n + 2) % 3];
    }
    else if (n < 2 * ny) {
      lattice.boundary_g[n] = nodes[(n + 2) % 3];
    }
    else if (n < 2 * ny + nx) {
      lattice.boundary_g[n] = nodes[(n + 1) % 3];
    }
    else if (n < 2 * ny + 2 * nx) {
      lattice.boundary_g[n] = nodes[n % 3];
    }
    else {
      lattice.boundary_g[n] = nodes[(n + 1) % 2];
    }
  }  // n
  lattice.g = lattice.Stream(lattice.g, lattice.boundary_g);
  for (auto y = 0u; y < ny; ++y) {
    for (auto x = 0u; x < nx; ++x) {
      auto n = y * nx + x;
      CHECK_CLOSE(result[(x % 3 + y % 3 + 1) % 3], lattice.g[n][NE], zero_tol);
      CHECK_CLOSE(result[(x % 3 + y % 3 + 2) % 3], lattice.g[n][SW], zero_tol);
    }  // x
  }  // y
}

TEST(StreamDiagonalNWSE)
{
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  std::vector<double> ones(9, 1);
  std::vector<double> twos(9, 2);
  std::vector<double> threes(9, 3);
  std::vector<std::vector<double>> nodes = {ones, twos, threes};
  std::vector<double> result = {1, 2, 3};
  for (auto y = 0u; y < ny; ++y) {
    for (auto x = 0u; x < nx; ++x) {
      auto n = y * nx + x;
      lattice.g[n] = nodes[(2 - x % 3 + y % 3) % 3];
    }  // x
  }  // y
  lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
  for (auto n = 0u; n < lattice.boundary_g.size(); ++n) {
    if (n < ny) {
      lattice.boundary_g[n] = nodes[n % 3];
    }
    else if (n < 2 * ny) {
      lattice.boundary_g[n] = nodes[(n + 2) % 3];
    }
    else if (n < 2 * ny + nx) {
      lattice.boundary_g[n] = nodes[(1 - n) % 3];
    }
    else if (n < 2 * ny + 2 * nx) {
      lattice.boundary_g[n] = nodes[(2 - n) % 3];
    }
    else {
      lattice.boundary_g[n] = nodes[2 * ((n + 1) % 2)];
    }
  }  // n
  lattice.g = lattice.Stream(lattice.g, lattice.boundary_g);
  for (auto y = 0u; y < ny; ++y) {
    for (auto x = 0u; x < nx; ++x) {
      auto n = y * nx + x;
      CHECK_CLOSE(result[(3 - x % 3 + y % 3) % 3], lattice.g[n][NW], zero_tol);
      CHECK_CLOSE(result[(4 - x % 3 + y % 3) % 3], lattice.g[n][SE], zero_tol);
    }  // x
  }  // y
}

TEST(CollideCDEWithSource)
{
  std::size_t nx = 1;
  std::size_t ny = 1;
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities, ny, nx, g_dx,
      g_dt, g_t_total, g_diffusion_coefficient, g_kinematic_viscosity,
      g_density_f, g_density_g, g_u0, g_src_pos_f, g_src_pos_g,
      g_src_strength_f, g_src_strength_g, g_obstacles_pos, g_is_cd, g_is_ns,
      g_is_not_instant, g_no_obstacles);
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  std::vector<double> g_exp1 = {0.114068,
                                1.005902, 1.914569, 2.822073, 3.730159,
                                4.635126, 5.543357, 6.451443, 7.359965};
  std::vector<double> g_exp2 = {0.032896,
                                1.006443, 1.998365, 2.989018, 3.980306,
                                4.96819, 5.959636, 6.950924, 7.942688};
  std::vector<double> u0 = {1.0, 2.0};
  std::vector<std::vector<unsigned>> src_pos = {{0, 0}};
  std::vector<double> src_strength = {50.5};
  std::vector<double> ones(nc, 1.0);
  std::vector<double> nums = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  lattice.Init(lattice.g, ones);
  lattice.Init(lattice.g_eq, nums);
  lattice.Init(lattice.u, u0);
  lattice.Init(lattice.src_g, src_pos, src_strength);
  // First collide
  lattice.Collide(lattice.g, lattice.g_eq, lattice.src_g);
  for (auto i = 0u; i < nc; ++i) {
    CHECK_CLOSE(g_exp1[i], lattice.g[0][i], loose_tol);
  }  // i
  // Second collide
  lattice.Collide(lattice.g, lattice.g_eq, lattice.src_g);
  for (auto i = 0u; i < nc; ++i) {
    CHECK_CLOSE(g_exp2[i], lattice.g[0][i], loose_tol);
  }  // i
}

TEST(CollideNSWithSource)
{
  std::size_t nx = 1;
  std::size_t ny = 1;
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities, ny,
      nx, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, g_u0, g_src_pos_f,
      g_src_pos_g, g_src_strength_f, g_src_strength_g, g_obstacles_pos, g_is_cd,
      g_is_ns, g_is_not_instant, g_no_obstacles);
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  std::vector<double> f_exp1 = {-0.15245685,
                                0.97186065, 2.00264452, 2.78578074, 3.81200867,
                                4.70034445, 5.53791136, 6.5145409, 7.35352415};
  std::vector<double> f_exp2 = {-0.258049,
                                0.9692824, 2.0945148, 2.9494004, 4.069655,
                                5.0393834, 5.9536912, 7.019803, 7.935657};
  std::vector<double> u0 = {234.0, 432.0};
  std::vector<std::vector<unsigned>> src_pos = {{0, 0}};
  std::vector<std::vector<double>> src_strength = {{123.0, 321.0}};
  std::vector<double> ones(nc, 1.0);
  std::vector<double> nums = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  lattice.Init(lattice.f, ones);
  lattice.Init(lattice.f_eq, nums);
  lattice.Init(lattice.u, u0);
  lattice.Init(lattice.rho_f, 2.0);
  lattice.Init(lattice.src_f, src_pos, src_strength);
  // First collide
  lattice.Collide(lattice.f, lattice.f_eq, lattice.src_f, lattice.rho_f);
  for (auto i = 0u; i < nc; ++i) {
    CHECK_CLOSE(f_exp1[i], lattice.f[0][i], loose_tol);
  }  // i
  // Second collide
  lattice.Collide(lattice.f, lattice.f_eq, lattice.src_f, lattice.rho_f);
  for (auto i = 0u; i < nc; ++i) {
    CHECK_CLOSE(f_exp2[i], lattice.f[0][i], loose_tol);
  }  // i
}

TEST(ComputeRho)
{
  Lattice lattice(g_lattice);
  std::vector<double> nums = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  double expected = 36.0;
  lattice.Init(lattice.g, nums);
  lattice.rho_g = lattice.ComputeRho(lattice.g);
  for (auto rho : lattice.rho_g) CHECK_CLOSE(expected, rho, loose_tol);
}

TEST(ComputeU)
{
  Lattice lattice(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  std::vector<std::vector<unsigned>> src_position((nx * ny), {0, 0});
  std::vector<std::vector<double>> src_strength((nx * ny), {123, 321});
  for (auto y = 0u; y < ny; ++y) {
    for (auto x = 0u; x < nx; ++x) {
      auto n = y * nx + x;
      src_position[n] = {x, y};
    }  // x
  }  // y
  std::vector<double> nums = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  std::vector<double> expected = {-31.5385, -94.6395};
  lattice.Init(lattice.f, nums);
  lattice.Init(lattice.rho_f, g_density_f);
  lattice.Init(lattice.src_f, src_position, src_strength);
  lattice.ComputeU(lattice.f, lattice.rho_f, lattice.src_f);
  for (auto u : lattice.u) {
    CHECK_CLOSE(expected[0], u[0], loose_tol);
    CHECK_CLOSE(expected[1], u[1], loose_tol);
  }  // u
}

TEST(InitAllAgainstManualInit)
{
  Lattice lattice(g_lattice);
  Lattice lattice2(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  auto nd = lattice.GetNumberOfDimensions();

  lattice.InitAll();
  lattice2.Init(lattice2.u, g_u0);
  lattice2.Init(lattice2.rho_f, g_density_f);
  lattice2.ComputeEq(lattice2.f_eq, lattice2.rho_f);
  lattice2.Init(lattice2.f, lattice2.f_eq);
  lattice2.Init(lattice2.src_f, g_src_pos_f, g_src_strength_f);
  lattice2.Init(lattice2.rho_g, g_density_g);
  lattice2.ComputeEq(lattice2.g_eq, lattice2.rho_g);
  lattice2.Init(lattice2.g, lattice2.g_eq);
  lattice2.Init(lattice2.src_g, g_src_pos_g, g_src_strength_g);
  for (auto n = 0u; n < nx * ny; ++n) {
    CHECK_CLOSE(lattice.rho_g[n], lattice2.rho_g[n], zero_tol);
    CHECK_CLOSE(lattice.rho_f[n], lattice2.rho_f[n], zero_tol);
    CHECK_CLOSE(lattice.src_g[n], lattice2.src_g[n], zero_tol);
    for (auto i = 0u; i < nc; ++i) {
      CHECK_CLOSE(lattice.g[n][i], lattice2.g[n][i], zero_tol);
      CHECK_CLOSE(lattice.g_eq[n][i], lattice2.g_eq[n][i], zero_tol);
      CHECK_CLOSE(lattice.f[n][i], lattice2.f[n][i], zero_tol);
      CHECK_CLOSE(lattice.f_eq[n][i], lattice2.f_eq[n][i], zero_tol);
    }  // i
    for (auto d = 0u; d < nd; ++d) {
      CHECK_CLOSE(lattice.u[n][d], lattice2.u[n][d], zero_tol);
      CHECK_CLOSE(lattice.src_f[n][d], lattice2.src_f[n][d], zero_tol);
    }  // d
  }  // n
}

TEST(TakeStepIndivFunctionCDENS)
{
  Lattice lattice(g_lattice);
  Lattice lattice2(g_lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  auto nd = lattice.GetNumberOfDimensions();

  lattice.InitAll();
  lattice2.InitAll();
  lattice.TakeStep();
  // NS
  lattice2.Collide(lattice2.f, lattice2.f_eq, lattice2.src_f, lattice2.rho_f);
  lattice2.boundary_f = lattice2.BoundaryCondition(lattice2.f);
  lattice2.f = lattice2.Stream(lattice2.f, lattice2.boundary_f);
  lattice2.rho_f = lattice2.ComputeRho(lattice2.f);
  lattice2.ComputeU(lattice2.f, lattice2.rho_f, lattice2.src_f);
  lattice2.ComputeEq(lattice2.f_eq, lattice2.rho_f);
  // CDE
  lattice2.Collide(lattice2.g, lattice2.g_eq, lattice2.src_g);
  lattice2.boundary_g = lattice2.BoundaryCondition(lattice2.g);
  lattice2.g = lattice2.Stream(lattice2.g, lattice2.boundary_g);
  lattice2.rho_g = lattice2.ComputeRho(lattice2.g);
  lattice2.ComputeEq(lattice2.g_eq, lattice2.rho_g);

  for (auto n = 0u; n < nx * ny; ++n) {
    CHECK_CLOSE(lattice.rho_g[n], lattice2.rho_g[n], zero_tol);
    CHECK_CLOSE(lattice.rho_f[n], lattice2.rho_f[n], zero_tol);
    for (auto i = 0u; i < nc; ++i) {
      CHECK_CLOSE(lattice.g[n][i], lattice2.g[n][i], zero_tol);
      CHECK_CLOSE(lattice.g_eq[n][i], lattice2.g_eq[n][i], zero_tol);
      CHECK_CLOSE(lattice.f[n][i], lattice2.f[n][i], zero_tol);
      CHECK_CLOSE(lattice.f_eq[n][i], lattice2.f_eq[n][i], zero_tol);
    }  // i
    for (auto d = 0u; d < nd; ++d) {
        CHECK_CLOSE(lattice.u[n][d], lattice2.u[n][d], zero_tol);
    }  // d
  }  // n
}

TEST(InstantSourceToggle)
{
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, g_u0, g_src_pos_f,
      g_src_pos_g, g_src_strength_f, g_src_strength_g, g_obstacles_pos, g_is_cd,
      g_is_ns, g_is_instant, g_no_obstacles);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  auto nd = lattice.GetNumberOfDimensions();
  lattice.RunSim();
  for (auto n = 0u; n < nx * ny; ++n) {
    CHECK_CLOSE(0.0, lattice.src_g[n], zero_tol);
    for (auto d = 0u; d < nd; ++d) {
      if (n == 0) {
        CHECK_CLOSE(1.0, lattice.src_f[n][d], zero_tol);
      }
      else {
        CHECK_CLOSE(0.0, lattice.src_f[n][d], zero_tol);
      }
    }  // d
  }  // n
}

TEST(ToggleCDE)
{
  std::vector<double> u0 = {1, 2};
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, u0, g_src_pos_f,
      g_src_pos_g, g_src_strength_f, g_src_strength_g, g_obstacles_pos, g_is_cd,
      g_is_not_ns, g_is_not_instant, g_no_obstacles);
  Lattice lattice2(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, u0, g_src_pos_f,
      g_src_pos_g, g_src_strength_f, g_src_strength_g, g_obstacles_pos, g_is_cd,
      g_is_not_ns, g_is_not_instant, g_no_obstacles);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  auto nd = lattice.GetNumberOfDimensions();
  lattice.RunSim();
  lattice2.Init(lattice2.u, u0);
  lattice2.Init(lattice2.rho_g, g_density_g);
  lattice2.ComputeEq(lattice2.g_eq, lattice2.rho_g);
  lattice2.Init(lattice2.g, lattice2.g_eq);
  lattice2.Init(lattice2.src_g, g_src_pos_g, g_src_strength_g);
  for (auto t = 0.0; t < g_t_total; t += g_dt) {
    lattice2.Collide(lattice2.g, lattice2.g_eq, lattice2.src_g);
    lattice2.boundary_g = lattice2.BoundaryCondition(lattice2.g);
    lattice2.g = lattice2.Stream(lattice2.g, lattice2.boundary_g);
    lattice2.rho_g = lattice2.ComputeRho(lattice2.g);
    lattice2.ComputeEq(lattice2.g_eq, lattice2.rho_g);
  }
  for (auto n = 0u; n < nx * ny; ++n) {
    CHECK_CLOSE(lattice.rho_f[n], lattice2.rho_f[n], zero_tol);
    CHECK_CLOSE(lattice.rho_g[n], lattice2.rho_g[n], zero_tol);
    CHECK_CLOSE(lattice.src_g[n], lattice2.src_g[n], zero_tol);
    for (auto i = 0u; i < nc; ++i) {
      CHECK_CLOSE(lattice.g[n][i], lattice2.g[n][i], zero_tol);
      CHECK_CLOSE(lattice.g_eq[n][i], lattice2.g_eq[n][i], zero_tol);
      CHECK_CLOSE(lattice.f[n][i], lattice2.f[n][i], zero_tol);
      CHECK_CLOSE(lattice.f_eq[n][i], lattice2.f_eq[n][i], zero_tol);
      for (auto j = 0u; j < lattice.boundary_f.size(); ++j) {
        CHECK_CLOSE(lattice.boundary_f[j][i], lattice2.boundary_f[j][i],
            zero_tol);
        CHECK_CLOSE(lattice.boundary_g[j][i], lattice2.boundary_g[j][i],
            zero_tol);
      }  // j
    }  // i
    for (auto d = 0u; d < nd; ++d) {
      CHECK_CLOSE(lattice.u[n][d], lattice2.u[n][d], zero_tol);
      CHECK_CLOSE(lattice.src_f[n][d], lattice2.src_f[n][d], zero_tol);
    }  // d
  }  // n
}

TEST(ToggleNS)
{
  std::vector<double> u0 = {1, 2};
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, u0, g_src_pos_f,
      g_src_pos_g, g_src_strength_f, g_src_strength_g, g_obstacles_pos,
      g_is_not_cd, g_is_ns, g_is_not_instant, g_no_obstacles);
  Lattice lattice2(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, u0, g_src_pos_f,
      g_src_pos_g, g_src_strength_f, g_src_strength_g, g_obstacles_pos,
      g_is_not_cd, g_is_ns, g_is_not_instant, g_no_obstacles);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  auto nd = lattice.GetNumberOfDimensions();
  lattice.RunSim();
  lattice2.Init(lattice2.u, u0);
  lattice2.Init(lattice2.rho_f, g_density_f);
  lattice2.ComputeEq(lattice2.f_eq, lattice2.rho_f);
  lattice2.Init(lattice2.f, lattice2.f_eq);
  lattice2.Init(lattice2.src_f, g_src_pos_f, g_src_strength_f);
  for (auto t = 0.0; t < g_t_total; t += g_dt) {
    lattice2.Collide(lattice2.f, lattice2.f_eq, lattice2.src_f, lattice2.rho_f);
    lattice2.boundary_f = lattice2.BoundaryCondition(lattice2.f);
    lattice2.f = lattice2.Stream(lattice2.f, lattice2.boundary_f);
    lattice2.rho_f = lattice2.ComputeRho(lattice2.f);
    lattice2.ComputeU(lattice2.f, lattice2.rho_f, lattice2.src_f);
    lattice2.ComputeEq(lattice2.f_eq, lattice2.rho_f);
  }
  for (auto n = 0u; n < nx * ny; ++n) {
    CHECK_CLOSE(lattice.rho_f[n], lattice2.rho_f[n], zero_tol);
    CHECK_CLOSE(lattice.rho_g[n], lattice2.rho_g[n], zero_tol);
    CHECK_CLOSE(lattice.src_g[n], lattice2.src_g[n], zero_tol);
    for (auto i = 0u; i < nc; ++i) {
      CHECK_CLOSE(lattice.g[n][i], lattice2.g[n][i], zero_tol);
      CHECK_CLOSE(lattice.g_eq[n][i], lattice2.g_eq[n][i], zero_tol);
      CHECK_CLOSE(lattice.f[n][i], lattice2.f[n][i], zero_tol);
      CHECK_CLOSE(lattice.f_eq[n][i], lattice2.f_eq[n][i], zero_tol);
      for (auto j = 0u; j < lattice.boundary_f.size(); ++j) {
        CHECK_CLOSE(lattice.boundary_f[j][i], lattice2.boundary_f[j][i],
            zero_tol);
        CHECK_CLOSE(lattice.boundary_g[j][i], lattice2.boundary_g[j][i],
            zero_tol);
      }  // j
    }  // i
    for (auto d = 0u; d < nd; ++d) {
      CHECK_CLOSE(lattice.u[n][d], lattice2.u[n][d], zero_tol);
      CHECK_CLOSE(lattice.src_f[n][d], lattice2.src_f[n][d], zero_tol);
    }  // d
  }  // n
}

TEST(ToggleObstacles)
{
  std::vector<double> u0 = {1, 2};
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, u0, g_src_pos_f,
      g_src_pos_g, g_src_strength_f, g_src_strength_g, g_obstacles_pos,
      g_is_cd, g_is_ns, g_is_not_instant, g_has_obstacles);
  Lattice lattice2(lattice);
  auto nx = lattice.GetNumberOfColumns();
  auto ny = lattice.GetNumberOfRows();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  auto nd = lattice.GetNumberOfDimensions();
  lattice.RunSim();
  lattice2.Init(lattice2.u, u0);
  lattice2.Init(lattice2.obstacles, g_obstacles_pos);
  lattice2.Init(lattice2.rho_f, g_density_f);
  lattice2.ComputeEq(lattice2.f_eq, lattice2.rho_f);
  lattice2.Init(lattice2.f, lattice2.f_eq);
  lattice2.Init(lattice2.src_f, g_src_pos_f, g_src_strength_f);
  lattice2.Init(lattice2.rho_g, g_density_g);
  lattice2.ComputeEq(lattice2.g_eq, lattice2.rho_g);
  lattice2.Init(lattice2.g, lattice2.g_eq);
  lattice2.Init(lattice2.src_g, g_src_pos_g, g_src_strength_g);
  for (auto t = 0.0; t < g_t_total; t += g_dt) {
    lattice2.Collide(lattice2.f, lattice2.f_eq, lattice2.src_f, lattice2.rho_f);
    lattice2.Obstacles(lattice2.f, lattice2.obstacles);
    lattice2.boundary_f = lattice2.BoundaryCondition(lattice2.f);
    lattice2.f = lattice2.Stream(lattice2.f, lattice2.boundary_f);
    lattice2.rho_f = lattice2.ComputeRho(lattice2.f);
    lattice2.ComputeU(lattice2.f, lattice2.rho_f, lattice2.src_f);
    lattice2.ComputeEq(lattice2.f_eq, lattice2.rho_f);
    lattice2.Collide(lattice2.g, lattice2.g_eq, lattice2.src_g);
    lattice2.Obstacles(lattice2.g, lattice2.obstacles);
    lattice2.boundary_g = lattice2.BoundaryCondition(lattice2.g);
    lattice2.g = lattice2.Stream(lattice2.g, lattice2.boundary_g);
    lattice2.rho_g = lattice2.ComputeRho(lattice2.g);
    lattice2.ComputeEq(lattice2.g_eq, lattice2.rho_g);
  }
  for (auto n = 0u; n < nx * ny; ++n) {
    CHECK_CLOSE(lattice.rho_f[n], lattice2.rho_f[n], zero_tol);
    CHECK_CLOSE(lattice.rho_g[n], lattice2.rho_g[n], zero_tol);
    CHECK_CLOSE(lattice.src_g[n], lattice2.src_g[n], zero_tol);
    for (auto i = 0u; i < nc; ++i) {
      CHECK_CLOSE(lattice.g[n][i], lattice2.g[n][i], zero_tol);
      CHECK_CLOSE(lattice.g_eq[n][i], lattice2.g_eq[n][i], zero_tol);
      CHECK_CLOSE(lattice.f[n][i], lattice2.f[n][i], zero_tol);
      CHECK_CLOSE(lattice.f_eq[n][i], lattice2.f_eq[n][i], zero_tol);
      for (auto j = 0u; j < lattice.boundary_f.size(); ++j) {
        CHECK_CLOSE(lattice.boundary_f[j][i], lattice2.boundary_f[j][i],
            zero_tol);
        CHECK_CLOSE(lattice.boundary_g[j][i], lattice2.boundary_g[j][i],
            zero_tol);
      }  // j
    }  // i
    for (auto d = 0u; d < nd; ++d) {
      CHECK_CLOSE(lattice.u[n][d], lattice2.u[n][d], zero_tol);
      CHECK_CLOSE(lattice.src_f[n][d], lattice2.src_f[n][d], zero_tol);
    }  // d
  }  // n
}

TEST(ObstaclesDoNotReflectIntoObstacles)
{
  std::vector<std::vector<unsigned>> obstacles_pos;
  std::vector<double> u0 = {0, 0};
  for (auto y = 1u; y < 4u; ++y) {
    obstacles_pos.push_back({1, y});
    obstacles_pos.push_back({2, y});
    obstacles_pos.push_back({3, y});
  }
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, u0, g_src_pos_f,
      g_src_pos_g, g_src_strength_f, g_src_strength_g, obstacles_pos,
      g_is_cd, g_is_ns, g_is_not_instant, g_has_obstacles);
  lattice.InitAll();
  for (auto y = 1u; y < 4u; ++y) {
    auto n = y * g_num_cols;
    CHECK_EQUAL(true, lattice.obstacles[n + 1]);
    CHECK_EQUAL(true, lattice.obstacles[n + 2]);
    CHECK_EQUAL(true, lattice.obstacles[n + 3]);
  }
  unsigned obs_center = 2 * g_num_cols + 2;
  for (auto t = 0; t < 1000; ++t) {
    lattice.Collide(lattice.f, lattice.f_eq, lattice.src_f, lattice.rho_f);
    auto node_f = lattice.f[obs_center];
    lattice.Obstacles(lattice.f, lattice.obstacles);
    for (auto i = 1u; i < g_num_discrete_velocities; ++i) {
      CHECK_CLOSE(node_f[i], lattice.f[obs_center][i], zero_tol);
    }
    lattice.boundary_f = lattice.BoundaryCondition(lattice.f);
    lattice.f = lattice.Stream(lattice.f, lattice.boundary_f);
    lattice.rho_f = lattice.ComputeRho(lattice.f);
    lattice.ComputeU(lattice.f, lattice.rho_f, lattice.src_f);
    lattice.ComputeEq(lattice.f_eq, lattice.rho_f);
    lattice.Collide(lattice.g, lattice.g_eq, lattice.src_g);
    auto node_g = lattice.g[obs_center];
    lattice.Obstacles(lattice.g, lattice.obstacles);
    for (auto i = 1u; i < g_num_discrete_velocities; ++i) {
      CHECK_CLOSE(node_g[i], lattice.g[obs_center][i], zero_tol);
    }
    lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
    lattice.g = lattice.Stream(lattice.g, lattice.boundary_g);
    lattice.rho_g = lattice.ComputeRho(lattice.g);
    lattice.ComputeEq(lattice.g_eq, lattice.rho_g);
  }
}

TEST(ObstaclesDoNotReflectLeftRightEdges)
{
  std::vector<std::vector<unsigned>> obstacles_pos;
  std::vector<double> u0 = {0, 0};
  std::vector<std::vector<unsigned>> src_pos_g = {{2, 2}};
  for (auto y = 0u; y < g_num_rows; ++y) {
    obstacles_pos.push_back({0, y});
    obstacles_pos.push_back({g_num_cols - 1, y});
  }
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, u0, g_src_pos_f,
      src_pos_g, g_src_strength_f, g_src_strength_g, obstacles_pos,
      g_is_cd, g_is_ns, g_is_not_instant, g_has_obstacles);
  lattice.InitAll();
  for (auto y = 0u; y < g_num_rows; ++y) {
    auto n = y * g_num_cols;
    CHECK_EQUAL(true, lattice.obstacles[n]);
    CHECK_EQUAL(true, lattice.obstacles[n + g_num_cols - 1]);
  }
  for (auto t = 0; t < 1000; ++t) {
    lattice.Collide(lattice.f, lattice.f_eq, lattice.src_f, lattice.rho_f);

    std::vector<std::vector<double>> left_edge_unchanged_f;
    std::vector<std::vector<double>> right_edge_unchanged_f;
    for (auto y = 0u; y < g_num_rows; ++y) {
      auto n = y * g_num_cols;
      left_edge_unchanged_f.push_back(lattice.f[n]);
      right_edge_unchanged_f.push_back(lattice.f[n + g_num_cols - 1]);
    }
    lattice.Obstacles(lattice.f, lattice.obstacles);

    for (auto y = 0u; y < g_num_rows; ++y) {
      auto n = y * g_num_cols;
      CHECK_CLOSE(left_edge_unchanged_f[y][W], lattice.f[n][W], zero_tol);
      CHECK_CLOSE(left_edge_unchanged_f[y][NW], lattice.f[n][NW], zero_tol);
      CHECK_CLOSE(left_edge_unchanged_f[y][SW], lattice.f[n][SW], zero_tol);
      CHECK_CLOSE(right_edge_unchanged_f[y][E],
          lattice.f[n + g_num_cols - 1][E], zero_tol);
      CHECK_CLOSE(right_edge_unchanged_f[y][NE],
          lattice.f[n + g_num_cols - 1][NE], zero_tol);
      CHECK_CLOSE(right_edge_unchanged_f[y][SE],
          lattice.f[n + g_num_cols - 1][SE], zero_tol);
    }
    lattice.boundary_f = lattice.BoundaryCondition(lattice.f);
    lattice.f = lattice.Stream(lattice.f, lattice.boundary_f);
    lattice.rho_f = lattice.ComputeRho(lattice.f);
    lattice.ComputeU(lattice.f, lattice.rho_f, lattice.src_f);
    lattice.ComputeEq(lattice.f_eq, lattice.rho_f);
    lattice.Collide(lattice.g, lattice.g_eq, lattice.src_g);

    std::vector<std::vector<double>> left_edge_unchanged_g;
    std::vector<std::vector<double>> right_edge_unchanged_g;
    for (auto y = 0u; y < g_num_rows; ++y) {
      auto n = y * g_num_cols;
      left_edge_unchanged_g.push_back(lattice.g[n]);
      right_edge_unchanged_g.push_back(lattice.g[n + g_num_cols - 1]);
    }
    lattice.Obstacles(lattice.g, lattice.obstacles);

    for (auto y = 0u; y < g_num_rows; ++y) {
      auto n = y * g_num_cols;
      CHECK_CLOSE(left_edge_unchanged_g[y][W], lattice.g[n][W], zero_tol);
      CHECK_CLOSE(left_edge_unchanged_g[y][NW], lattice.g[n][NW], zero_tol);
      CHECK_CLOSE(left_edge_unchanged_g[y][SW], lattice.g[n][SW], zero_tol);
      CHECK_CLOSE(right_edge_unchanged_g[y][E],
          lattice.g[n + g_num_cols - 1][E], zero_tol);
      CHECK_CLOSE(right_edge_unchanged_g[y][NE],
          lattice.g[n + g_num_cols - 1][NE], zero_tol);
      CHECK_CLOSE(right_edge_unchanged_g[y][SE],
          lattice.g[n + g_num_cols - 1][SE], zero_tol);
    }
    lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
    lattice.g = lattice.Stream(lattice.g, lattice.boundary_g);
    lattice.rho_g = lattice.ComputeRho(lattice.g);
    lattice.ComputeEq(lattice.g_eq, lattice.rho_g);
  }
}

TEST(ObstaclesDoNotReflectCorners)
{
  std::vector<std::vector<unsigned>> obstacles_pos = {{0, 0},
      {0, g_num_rows - 1}, {g_num_cols - 1, 0},
      {g_num_cols - 1, g_num_rows - 1}};
  std::vector<double> u0 = {0, 0};
  std::vector<std::vector<unsigned>> src_pos_g = {{2, 2}};
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, u0, g_src_pos_f,
      src_pos_g, g_src_strength_f, g_src_strength_g, obstacles_pos,
      g_is_cd, g_is_ns, g_is_not_instant, g_has_obstacles);
  lattice.InitAll();
  unsigned bottom_right = g_num_cols - 1;
  unsigned top_left = (g_num_rows - 1) * g_num_cols;
  unsigned top_right = (g_num_rows - 1) * g_num_cols + g_num_cols - 1;
  CHECK_EQUAL(true, lattice.obstacles[0]);
  CHECK_EQUAL(true, lattice.obstacles[bottom_right]);
  CHECK_EQUAL(true, lattice.obstacles[top_left]);
  CHECK_EQUAL(true, lattice.obstacles[top_right]);
  for (auto t = 0; t < 1000; ++t) {
    lattice.Collide(lattice.f, lattice.f_eq, lattice.src_f, lattice.rho_f);

    std::vector<std::vector<double>> corners_unchanged_f = {lattice.f[0],
        lattice.f[bottom_right], lattice.f[top_left], lattice.f[top_right]};
    lattice.Obstacles(lattice.f, lattice.obstacles);

    CHECK_CLOSE(corners_unchanged_f[0][SW], lattice.f[0][SW], zero_tol);
    CHECK_CLOSE(corners_unchanged_f[1][SE], lattice.f[bottom_right][SE],
        zero_tol);
    CHECK_CLOSE(corners_unchanged_f[2][NW], lattice.f[top_left][NW], zero_tol);
    CHECK_CLOSE(corners_unchanged_f[3][NE], lattice.f[top_right][NE], zero_tol);

    lattice.boundary_f = lattice.BoundaryCondition(lattice.f);
    lattice.f = lattice.Stream(lattice.f, lattice.boundary_f);
    lattice.rho_f = lattice.ComputeRho(lattice.f);
    lattice.ComputeU(lattice.f, lattice.rho_f, lattice.src_f);
    lattice.ComputeEq(lattice.f_eq, lattice.rho_f);
    lattice.Collide(lattice.g, lattice.g_eq, lattice.src_g);

    std::vector<std::vector<double>> corners_unchanged_g = {lattice.g[0],
        lattice.g[bottom_right], lattice.g[top_left], lattice.g[top_right]};
    lattice.Obstacles(lattice.g, lattice.obstacles);

    CHECK_CLOSE(corners_unchanged_g[0][SW], lattice.g[0][SW], zero_tol);
    CHECK_CLOSE(corners_unchanged_g[1][SE], lattice.g[bottom_right][SE],
        zero_tol);
    CHECK_CLOSE(corners_unchanged_g[2][NW], lattice.g[top_left][NW], zero_tol);
    CHECK_CLOSE(corners_unchanged_g[3][NE], lattice.g[top_right][NE], zero_tol);

    lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
    lattice.g = lattice.Stream(lattice.g, lattice.boundary_g);
    lattice.rho_g = lattice.ComputeRho(lattice.g);
    lattice.ComputeEq(lattice.g_eq, lattice.rho_g);
  }
}

// Exception Tests
TEST(ZeroValuesInLatticeDeclaration)
{
  std::vector<double> u0;
  CHECK_THROW(Lattice lattice(0, 9, 2, 4, 1, 1, 1, 0.2, 0.2, 2.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 0, 2, 4, 1, 1, 1, 0.2, 0.2, 2.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 0, 4, 1, 1, 1, 0.2, 0.2, 2.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 2, 0, 1, 1, 1, 0.2, 0.2, 2.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 2, 4, 0, 1, 1, 0.2, 0.2, 2.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 2, 4, 1, 0, 1, 0.2, 0.2, 2.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 2, 4, 1, 1, 0, 0.2, 0.2, 2.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 2, 4, 1, 1, 1, 0.0, 0.2, 2.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 2, 4, 1, 1, 1, 0.2, 0.0, 2.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 2, 4, 1, 1, 1, 0.2, 0.2, 0.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 2, 4, 1, 1, 1, 0.2, 0.2, 2.0, 0.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 2, 4, 1, 1, 1, 0.2, 0.2, 2.0, 2.0, u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, true, true, true, false), std::runtime_error);
  CHECK_THROW(Lattice lattice(2, 9, 2, 4, 1, 1, 1, 0.2, 0.2, 2.0, 2.0, g_u0,
      g_src_pos_f, g_src_pos_g, g_src_strength_f, g_src_strength_g,
      g_obstacles_pos, false, false, true, false), std::runtime_error);
}

TEST(Init2DLatticesWithValuesWrongDimensions)
{
  Lattice lattice(g_lattice);
  std::vector<double> initial_u = {1.0, 2.0, 3.0};
  CHECK_THROW(lattice.Init(lattice.u, initial_u), std::runtime_error);
}

TEST(Init2DLatticesWithLatticeWrongDimensions)
{
  Lattice lattice(g_lattice);
  auto ny = lattice.GetNumberOfRows();
  auto nx = lattice.GetNumberOfColumns();
  std::vector<double> node = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector<std::vector<double>> initial_g(nx * (ny + 2), node);
  CHECK_THROW(lattice.Init(lattice.g, initial_g), std::runtime_error);
}

TEST(Init2DLatticesWithLatticeWrongDepth)
{
  Lattice lattice(g_lattice);
  auto ny = lattice.GetNumberOfRows();
  auto nx = lattice.GetNumberOfColumns();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  for (auto n = 0u; n < 100; ++n) {
    if (n == nc) continue;
    std::vector<double> node(n, 0.0);
    std::vector<std::vector<double>> initial_g(nx * ny, node);
    CHECK_THROW(lattice.Init(lattice.g, initial_g), std::runtime_error);
  }
}

TEST(InitPosStrengthMismatch)
{
  std::vector<std::vector<unsigned>> src_pos_g = {{1, 2}};
  std::vector<double> src_strength_g = {1.0, 1.5};
  std::vector<std::vector<unsigned>> src_pos_f = {{1, 2}};
  std::vector<std::vector<double>> src_strength_f = {{1, 2}, {2, 2}};
  Lattice lattice(g_lattice);
  CHECK_THROW(lattice.Init(lattice.src_g, src_pos_g, src_strength_g),
      std::runtime_error);
  CHECK_THROW(lattice.Init(lattice.src_f, src_pos_f, src_strength_f),
      std::runtime_error);
}

TEST(InitWrongPositionLength)
{
  std::vector<std::vector<unsigned>> src_pos_long = {{1, 2, 3}};
  std::vector<double> src_strength_g = {1.0};
  std::vector<std::vector<unsigned>> src_pos_short = {{1}};
  std::vector<std::vector<double>> src_strength_f = {{1, 2}};
  Lattice lattice(g_lattice);
  CHECK_THROW(lattice.Init(lattice.src_g, src_pos_long, src_strength_g),
      std::runtime_error);
  CHECK_THROW(lattice.Init(lattice.src_g, src_pos_short, src_strength_g),
      std::runtime_error);
  CHECK_THROW(lattice.Init(lattice.src_f, src_pos_long, src_strength_f),
      std::runtime_error);
  CHECK_THROW(lattice.Init(lattice.src_f, src_pos_short, src_strength_f),
      std::runtime_error);
  CHECK_THROW(lattice.Init(lattice.obstacles, src_pos_long),
      std::runtime_error);
  CHECK_THROW(lattice.Init(lattice.obstacles, src_pos_short),
      std::runtime_error);
}

TEST(InitXPostionOutOfBound)
{
  std::vector<std::vector<unsigned>> src_pos = {{10, 2}};
  std::vector<double> src_strength_g = {1.0};
  std::vector<std::vector<double>> src_strength_f = {{1, 2}};
  Lattice lattice(g_lattice);
  CHECK_THROW(lattice.Init(lattice.src_g, src_pos, src_strength_g),
      std::runtime_error);
  CHECK_THROW(lattice.Init(lattice.src_f, src_pos, src_strength_f),
      std::runtime_error);
  CHECK_THROW(lattice.Init(lattice.obstacles, src_pos), std::runtime_error);
}

TEST(InitYPostionOutOfBound)
{
  std::vector<std::vector<unsigned>> src_pos = {{1, 20}};
  std::vector<double> src_strength_g = {1.0};
  std::vector<std::vector<double>> src_strength_f = {{1, 2}};
  Lattice lattice(g_lattice);
  CHECK_THROW(lattice.Init(lattice.src_g, src_pos, src_strength_g),
      std::runtime_error);
  CHECK_THROW(lattice.Init(lattice.src_f, src_pos, src_strength_f),
      std::runtime_error);
  CHECK_THROW(lattice.Init(lattice.obstacles, src_pos), std::runtime_error);
}

TEST(CollideWrongLatticeSize)
{
  Lattice lattice(g_lattice);
  auto ny = lattice.GetNumberOfRows();
  auto nx = lattice.GetNumberOfColumns();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  std::vector<std::vector<double>> smaller_lattice((nx - 1) * (ny - 1),
      std::vector<double>(nc, 1));
  lattice.InitAll();
  CHECK_THROW(lattice.Collide(lattice.g, smaller_lattice, lattice.src_g),
      std::runtime_error);
  CHECK_THROW(lattice.Collide(lattice.f, smaller_lattice, lattice.src_f,
      lattice.rho_f), std::runtime_error);
}

TEST(PrintWrongDepth)
{
  Lattice lattice(g_lattice);
  auto ny = lattice.GetNumberOfRows();
  auto nx = lattice.GetNumberOfColumns();
  auto nc = lattice.GetNumberOfDiscreteVelocities();
  lattice.InitAll();
  std::vector<std::vector<double>> shallow_boundary((2 * nx + 2 * ny + 4),
      std::vector<double>(nc - 1, 1));
  CHECK_THROW(lattice.Print(0, lattice.u), std::runtime_error);
  CHECK_THROW(lattice.Print(1, lattice.g), std::runtime_error);
  CHECK_THROW(lattice.Print(2, shallow_boundary), std::runtime_error);
  CHECK_THROW(lattice.Print(4, lattice.g), std::runtime_error);
}

TEST(ObstacleNormalReflect)
{
  auto m = 2u;
  auto n = 2u;
  std::vector<std::vector<unsigned>> obs_pos(g_num_rows * g_num_cols, {m, n});
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, g_u0_zero, g_src_pos_f,
      g_src_pos_g, g_src_strength_f, g_src_strength_g, obs_pos,
      g_is_not_cd, g_is_ns, g_is_not_instant, g_obstacles);
  auto ny = lattice.GetNumberOfRows();
  lattice.InitAll();
  lattice.TakeStep();
  lattice.Obstacles(lattice.g, lattice.obstacles);
  CHECK_CLOSE(lattice.g[m + ny * n][N], lattice.g[m + ny * (n + 1)][S],
      zero_tol);
  CHECK_CLOSE(lattice.g[m + ny * n][S], lattice.g[m + ny * (n - 1)][N],
      zero_tol);
  CHECK_CLOSE(lattice.g[m + ny * n][E], lattice.g[(m + 1) + ny * n][W],
      zero_tol);
  CHECK_CLOSE(lattice.g[m + ny * n][W], lattice.g[(m - 1) + ny * n][E],
      zero_tol);
  CHECK_CLOSE(lattice.g[m + ny * n][NE], lattice.g[(m + 1) + ny * (n + 1)][SW],
      zero_tol);
  CHECK_CLOSE(lattice.g[m + ny * n][NW], lattice.g[(m - 1) + ny * (n + 1)][SE],
      zero_tol);
  CHECK_CLOSE(lattice.g[m + ny * n][SE], lattice.g[(m + 1) + ny * (n - 1)][NW],
      zero_tol);
  CHECK_CLOSE(lattice.g[m + ny * n][SW], lattice.g[(m - 1) + ny * (n - 1)][NE],
      zero_tol);
}

TEST(TopBottomNoReflect)
{
  std::vector<std::vector<unsigned>> obstacles_pos;
  std::vector<double> u0 = {0, 0};
  std::vector<std::vector<unsigned>> src_pos_g = {{2, 2}};
  for (auto x = 0u; x < g_num_cols; ++x) {
    obstacles_pos.push_back({x, 0});
    obstacles_pos.push_back({x, g_num_rows - 1});
  }
  Lattice lattice(g_num_dimensions, g_num_discrete_velocities,
      g_num_rows, g_num_cols, g_dx, g_dt, g_t_total, g_diffusion_coefficient,
      g_kinematic_viscosity, g_density_f, g_density_g, u0, g_src_pos_f,
      src_pos_g, g_src_strength_f, g_src_strength_g, obstacles_pos,
      g_is_cd, g_is_ns, g_is_not_instant, g_has_obstacles);
  lattice.InitAll();
  unsigned top_row = g_num_cols * (g_num_rows - 1);
  for (auto x = 0u; x < g_num_cols; ++x) {
    CHECK_EQUAL(true, lattice.obstacles[x]);
    CHECK_EQUAL(true, lattice.obstacles[x + top_row]);
  }
  for (auto t = 0; t < 1000; ++t) {
    lattice.Collide(lattice.f, lattice.f_eq, lattice.src_f, lattice.rho_f);
    std::vector<std::vector<double>> top_edge_unchanged_f;
    std::vector<std::vector<double>> bottom_edge_unchanged_f;
    for (auto x = 0u; x < g_num_cols; ++x) {
      bottom_edge_unchanged_f.push_back(lattice.f[x]);
      top_edge_unchanged_f.push_back(lattice.f[x + top_row]);
    }
    lattice.Obstacles(lattice.f, lattice.obstacles);
    for (auto x = 0u; x < g_num_cols; ++x) {
      CHECK_CLOSE(top_edge_unchanged_f[x][N],
          lattice.f[x + top_row][N], zero_tol);
      CHECK_CLOSE(top_edge_unchanged_f[x][NW],
          lattice.f[x + top_row][NW], zero_tol);
      CHECK_CLOSE(top_edge_unchanged_f[x][NE],
          lattice.f[x + top_row][NE], zero_tol);
      CHECK_CLOSE(bottom_edge_unchanged_f[x][S], lattice.f[x][S], zero_tol);
      CHECK_CLOSE(bottom_edge_unchanged_f[x][SW], lattice.f[x][SW], zero_tol);
      CHECK_CLOSE(bottom_edge_unchanged_f[x][SE], lattice.f[x][SE], zero_tol);
    }
    lattice.boundary_f = lattice.BoundaryCondition(lattice.f);
    lattice.f = lattice.Stream(lattice.f, lattice.boundary_f);
    lattice.rho_f = lattice.ComputeRho(lattice.f);
    lattice.ComputeU(lattice.f, lattice.rho_f, lattice.src_f);
    lattice.ComputeEq(lattice.f_eq, lattice.rho_f);
    lattice.Collide(lattice.g, lattice.g_eq, lattice.src_g);
    std::vector<std::vector<double>> top_edge_unchanged_g;
    std::vector<std::vector<double>> bottom_edge_unchanged_g;
    for (auto x = 0u; x < g_num_cols; ++x) {
      bottom_edge_unchanged_g.push_back(lattice.g[x]);
      top_edge_unchanged_g.push_back(lattice.g[x + top_row]);
    }
    lattice.Obstacles(lattice.g, lattice.obstacles);
    for (auto x = 0u; x < g_num_cols; ++x) {
      CHECK_CLOSE(top_edge_unchanged_g[x][N],
          lattice.g[x + top_row][N], zero_tol);
      CHECK_CLOSE(top_edge_unchanged_g[x][NW],
          lattice.g[x + top_row][NW], zero_tol);
      CHECK_CLOSE(top_edge_unchanged_g[x][NE],
          lattice.g[x + top_row][NE], zero_tol);
      CHECK_CLOSE(bottom_edge_unchanged_g[x][S], lattice.g[x][S], zero_tol);
      CHECK_CLOSE(bottom_edge_unchanged_g[x][SW], lattice.g[x][SW], zero_tol);
      CHECK_CLOSE(bottom_edge_unchanged_g[x][SE], lattice.g[x][SE], zero_tol);
    }
    lattice.boundary_g = lattice.BoundaryCondition(lattice.g);
    lattice.g = lattice.Stream(lattice.g, lattice.boundary_g);
    lattice.rho_g = lattice.ComputeRho(lattice.g);
    lattice.ComputeEq(lattice.g_eq, lattice.rho_g);
  }
}
}  // suite FunctionalityAndExceptionsTests
