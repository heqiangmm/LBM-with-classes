#include "LatticeD2Q9.hpp"
#include "LatticeModel.hpp"
#include <iostream>
LatticeD2Q9::LatticeD2Q9(std::size_t num_rows
  , std::size_t num_cols
  , double dx
  , double dt)
{
  // can't use initializer list because inheritance provides access but doesn't
  // create member variables in derived class, so have to reassign them values
  // the old fashioned way
  // https://stackoverflow.com/questions/6986798/subtle-c-inheritance-error-with
  // -protected-fields
  number_of_dimensions_ = 2;
  number_of_directions_ = 9;
  number_of_rows_ = num_rows;
  number_of_columns_ = num_cols;
  space_step_ = dx;
  time_step_ = dt;
  c_ = space_step_ / time_step_;
  e = {{0, 0},
       {1, 0}, {0, 1}, {-1, 0}, {0, -1},
       {1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
  omega = {16.0 / 36.0,
           4.0 / 36.0, 4.0 / 36.0, 4.0 / 36.0, 4.0 / 36.0,
           1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0};
}

double LatticeD2Q9::GetZerothMoment(const std::vector<double> &node)
{
  double result = 0.0;
  for (auto i : node) result += i;
  return result;
}

std::vector<double> LatticeD2Q9::GetFirstMoment(const std::vector<double> &node)
{
  std::vector<double> result(2, 0.0);
  auto it_node = begin(node);
  for (auto dir : e) {
    result[0] += *it_node * dir[0] * c_;
    result[1] += (*it_node++) * dir[1] * c_;
  }
  return result;
}

std::vector<std::vector<double>> LatticeD2Q9::ComputeU(
      const std::vector<std::vector<double>> &lattice
    , const std::vector<double> &rho
    , const std::vector<std::vector<double>> &src)
{
  std::vector<std::vector<double>> result;
  auto index = 0u;
  for (auto node : lattice) {
    result.push_back(LatticeD2Q9::GetFirstMoment(node));
    result[index][0] += 0.5 * time_step_ * src[index][0] * rho[index];
    result[index][0] /= rho[index];
    result[index][1] += 0.5 * time_step_ * src[index][1] * rho[index];
    result[index][1] /= rho[index];
    ++index;
  }
  return result;
}