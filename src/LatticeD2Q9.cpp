#include "LatticeD2Q9.hpp"
#include <iostream>
#include <vector>
#include "Algorithm.hpp"
#include "LatticeModel.hpp"

LatticeD2Q9::LatticeD2Q9(std::size_t num_rows
  , std::size_t num_cols
  , double dx
  , double dt
  , const std::vector<double> &initial_velocity)
  : LatticeModel(2, 9, num_rows, num_cols, dx, dt, initial_velocity)
{
  // can't use initializer list because inheritance provides access but doesn't
  // create member variables in derived class, so have to call LatticeModel
  // constructor
  // https://stackoverflow.com/questions/6986798/subtle-c-inheritance-error-with
  // -protected-fields

  // cannot pass e_d2q9 to LatticeModel and initialize it in LatticeModel
  // initializer list so have to do it here.
  e = {{0, 0},
       {1, 0}, {0, 1}, {-1, 0}, {0, -1},
       {1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
  // set e to contain values of c_ in it according to "Viscous flow computations
  // with the method of lattice Boltzmann equation
  for (auto &i : e) {
    for (auto &d : i) d *= c_;
  }  // i
  omega = {16.0 / 36.0,
           4.0 / 36.0, 4.0 / 36.0, 4.0 / 36.0, 4.0 / 36.0,
           1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0};
}

LatticeD2Q9::LatticeD2Q9(std::size_t num_rows
  , std::size_t num_cols
  , double dx
  , double dt
  , const std::vector<std::vector<double>> &initial_velocity)
  : LatticeModel(2, 9, num_rows, num_cols, dx, dt, initial_velocity)
{
  e = {{0, 0},
       {1, 0}, {0, 1}, {-1, 0}, {0, -1},
       {1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
  for (auto &i : e) {
    for (auto &d : i) d *= c_;
  }  // i
  omega = {16.0 / 36.0,
           4.0 / 36.0, 4.0 / 36.0, 4.0 / 36.0, 4.0 / 36.0,
           1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0};
}
