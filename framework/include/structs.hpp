#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <map>
#include <glbinding/gl/gl.h>
#include <glm/gtc/type_ptr.hpp>
// use gl definitions from glbinding 
using namespace gl;

// gpu representation of model
struct model_object {
  // vertex array object
  GLuint vertex_AO = 0;
  // vertex buffer object
  GLuint vertex_BO = 0;
  // index buffer object
  GLuint element_BO = 0;
  // primitive type to draw
  GLenum draw_mode = GL_NONE;
  // indices number, if EBO exists
  GLsizei num_elements = 0;
};

// gpu representation of texture
struct texture_object {
  // handle of texture object
  GLuint handle = 0;
  // binding point
  GLenum target = GL_NONE;
};

// shader handle and uniform storage
struct shader_program {
  shader_program(std::string const& vertex, std::string const& fragment)
   :vertex_path{vertex}
   ,fragment_path{fragment}
   ,handle{0}
   {}

  // path to shader source
  std::string vertex_path; 
  std::string fragment_path; 
  // object handle
  GLuint handle;
  // uniform locations mapped to name
  std::map<std::string, GLint> u_locs{};
};

// planet struct with values size, rotation speed, distance & name
struct planet {
  planet(float const& size, float const& rotation_speed, float const& distance, glm::fvec3 const& color, int tex)
   :size_{size}
   ,rotation_speed_{rotation_speed}
   ,distance_{distance}
   ,color_{color}
   ,tex_{tex}
   ,tex_obj_{}
   {}

  float size_;
  //rotation per time unit around sun
  float rotation_speed_;
  //distance from center (sun)
  float distance_;
  //color
  glm::fvec3 color_;
  //texture
  int tex_;
  //texture object
  texture_object tex_obj_;
};

  struct moon : public planet {
  moon(float const& size, float const& rotation_speed, float const& distance, glm::fvec3 const& color, int tex, std::string const& planet_name)
   :planet{size, rotation_speed, distance, color, tex}
   ,planet_name_{planet_name}{}

  std::string planet_name_;
};
#endif