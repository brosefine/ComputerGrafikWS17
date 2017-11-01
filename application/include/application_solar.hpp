#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"
#include <map>
#include <vector>

// gpu representation of model
class ApplicationSolar : public Application {
 public:
  // allocate and initialize objects
  ApplicationSolar(std::string const& resource_path);
  // free allocated objects
  ~ApplicationSolar();

  // update uniform locations and values
  void uploadUniforms();
  // update projection matrix
  void updateProjection();
  // react to key input
  void keyCallback(int key, int scancode, int action, int mods);
  //handle delta mouse movement input
  void mouseCallback(double pos_x, double pos_y);

  //calculate and upload model-/normal-matrix for given planet
  void upload_planet_transforms(planet const& planet) const;

    //calculate and upload model-/normal-matrix for given moon
  void upload_moon_transforms(moon const& moon) const;

  // draw all objects
  void render() const;

  //add x stars with random data
  void addStars(unsigned int x);

 protected:
  void initializeShaderPrograms();
  void initializeGeometry();
  void updateView();

  // cpu representation of model
  model_object planet_object;
  model_object star_object;
  // planet, moon and star representations
  std::map<std::string, planet> planets;
  std::map<std::string, moon> moons;
  std::vector<float> stars;

};

#endif