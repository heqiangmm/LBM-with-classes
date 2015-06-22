#ifndef COLLISIONCD_HPP_
#define COLLISIONCD_HPP_
#include <vector>
#include "Collision.hpp"
#include "LatticeModel.hpp"

class CollisionCD: public Collision {
 public:
  /** \brief
   *
   * \param lat std::vector<std::vector<double>>&
   *
   */
  CollisionCD(LatticeModel &lm
    , const std::vector<std::vector<std::size_t>> &position
    , const std::vector<double> &strength
    , double diffusion_coefficient
    , double initial_density_g
    , const std::vector<double> &initial_velocity);

  /** \brief
   *
   * \param
   * \param
   * \return
   *
   */
  void InitSource(
      const std::vector<std::vector<std::size_t>> &position
    , const std::vector<double> &strength);

  /** \brief
   *
   * \return void
   *
   */
  void ApplyForce();
  // only public for ease of debugging with Print() function
  std::vector<double> source_;
 private:
//  std::vector<double> source_;
};
#endif  // COLLISIONCD_HPP_
