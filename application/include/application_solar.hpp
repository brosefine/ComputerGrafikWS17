#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "pixel_data.hpp"
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
  void upload_sun_transforms(planet const& sun) const;


  //calculate and upload model-/normal-matrix for given moon
  void upload_moon_transforms(moon const& moon) const;

  //calculate and upload model matrix for given planets or moons
  void upload_orbit_transforms(planet const& planet) const;
  void upload_orbit_transforms(moon const& moon) const;

  //color to shader
  void upload_color(glm::fvec3 const& color) const;
  //tex to shader
  void upload_texture(planet const& planet) const;

  // draw all objects
  void render() const;
  void renderSky() const;
  void renderSquad() const;

  //add x stars with random data
  void addStars(unsigned int x);
  //bind shaders and update matrices for all 
  void glUniform();
  //initialze orbit 
  void initializeOrbit();
  void initializeSquad();
  //initialize textures
  void initializeTextures();


 protected:
  void initializeShaderPrograms();
  void initializeGeometry();
  void initializeFramebufferHandles();
  void initializeFramebuffer();
  void initializeUniformBuffers();
  void updateView();

  // cpu representation of model
  model_object planet_object;
  model_object star_object;
  model_object orbit_object;
  model_object squad_object;

  //object handles for offscreen rendering
  texture_object m_texture_object;
  texture_object m_renderbuffer_object;
  texture_object m_framebuffer_object;
  //object handles for godrays
  texture_object light_texture_object;
  texture_object light_renderbuffer_object;
  texture_object light_framebuffer_object;

  texture_object sky_texture_object;

  cameraBuffer m_cameraBuffer;
  lightBuffer m_lightBuffer;
  texture_object ubo_camera;
  texture_object ubo_light;

  // planet, moon, star and orbit representations
  std::map<std::string, planet> planets;
  std::map<std::string, moon> moons;
  std::vector<float> stars;
  std::vector<float> orbit;
  std::vector<float> squad;

  mutable glm::fmat4 model_matrix_sun;

  bool m_greyscale = false;
  bool m_mirrored_v = false;
  bool m_mirrored_h = false;
  bool m_blur = false;
  bool m_scatter = false;
  

  //used shader
  mutable std::string shaderName = "planet_comic";

};

#endif