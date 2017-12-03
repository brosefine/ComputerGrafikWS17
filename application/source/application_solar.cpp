#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cstdlib>
#include <vector>
#include <math.h>

const glm::fvec3 sun            {1.0f, 0.98039f, 0.75f};
const glm::fvec3 merkur         {0.69412f, 0.67843f, 0.67843f};
const glm::fvec3 venus          {0.89020f, 0.73333f, 0.46275};
const glm::fvec3 earth          {0.41961f, 0.57647f, 0.83922f};
const glm::fvec3 mars           {0.75686f, 0.26667f, 0.05490f};
const glm::fvec3 jupiter        {0.80784f, 0.64706f, 0.53725f};
const glm::fvec3 saturn         {0.76471f, 0.57255f, 0.30980f};
const glm::fvec3 uranus         {0.38431f, 0.68235f, 0.90588f};
const glm::fvec3 neptune        {0.23922f, 0.36863f, 0.97647f};
const glm::fvec3 grey           {0.5f, 0.5f, 0.5f};

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,star_object{}
 ,orbit_object{}
 ,planets{}
 ,moons{}
 ,stars{}
 ,orbit{}
 ,textures{}
{
  //add planets and moon
  planets.insert(std::pair<std::string, planet>("sun", {1.0f, 0.0f, 0.0f, sun, 1}));
  planets.insert(std::pair<std::string, planet>("mercury", {0.25f, 1.0f, 3.0f, merkur, 2}));
  planets.insert(std::pair<std::string, planet>("venus", {0.25f, 0.74f, 5.0f, venus, 3}));
  planets.insert(std::pair<std::string, planet>("earth", {0.25f, 0.63f, 7.0f, earth, 4}));
  planets.insert(std::pair<std::string, planet>("mars", {0.25f, 0.51f, 9.0f, mars, 5}));
  planets.insert(std::pair<std::string, planet>("jupiter", {0.5f, 0.27f, 11.0f, jupiter, 6}));
  planets.insert(std::pair<std::string, planet>("saturn", {0.5f, 0.20f, 13.0f, saturn, 7}));
  planets.insert(std::pair<std::string, planet>("uranus", {0.3f, 0.14f, 15.0f, uranus, 8}));
  planets.insert(std::pair<std::string, planet>("neptune", {0.3f, 0.11f, 17.0f, neptune, 9}));
  moons.insert(std::pair<std::string, moon>("moon", {0.04f, 6.0f, 1.0f, "earth", grey}));

  //add stars
  addStars(1500);
  //add orbits
  initializeOrbit();
  //add textures
  initializeTextures();
  initializeGeometry();
  initializeShaderPrograms();
  
}


void ApplicationSolar::render() const {
  // bind shader to upload uniforms

  glBindVertexArray(star_object.vertex_AO);

  glUseProgram(m_shaders.at("stars").handle);
  glDrawArrays(star_object.draw_mode, NULL, star_object.num_elements);  

  for(auto& i : planets){
    //render planet
    upload_orbit_transforms(i.second);

    glBindVertexArray(orbit_object.vertex_AO);
    glDrawArrays(orbit_object.draw_mode, NULL, orbit_object.num_elements);

    if(i.first == "sun"){
      upload_sun_transforms(i.second);
    } else {    
      upload_planet_transforms(i.second);
    }
    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }

  for(auto& i : moons){
    //render planet
    upload_orbit_transforms(i.second);

    glBindVertexArray(orbit_object.vertex_AO);
    glDrawArrays(orbit_object.draw_mode, NULL, orbit_object.num_elements); 

    upload_moon_transforms(i.second);

    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }

   glBindVertexArray(0);

}

void ApplicationSolar::updateView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUniform("ViewMatrix", view_matrix);
}

void ApplicationSolar::upload_moon_transforms(moon const& moon) const{
  //get moons planet
  auto iter = planets.find(moon.planet_);
  if(iter == planets.end()){
    return;
  }
  planet planet = iter->second;

  //render and upload planets
  float s = moon.size_;
  float r = moon.rotation_speed_;
  float d = moon.distance_;

  float p_r = planet.rotation_speed_;
  float p_d = planet.distance_;

  //transform model matrix according to moon and planet attributes
  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(p_r*glfwGetTime()), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*p_d});
  model_matrix = glm::rotate(model_matrix, float(r*glfwGetTime()), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*d});
  model_matrix = glm::scale(model_matrix, glm::fvec3{s, s, s});

  //upload matrices to current shader
  glUseProgram(m_shaders.at(shaderName).handle);
  glUniformMatrix4fv(m_shaders.at(shaderName).u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at(shaderName).u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));
}

void ApplicationSolar::upload_planet_transforms(planet const& planet) const{
  //render and upload planets
  float s = planet.size_;
  float r = planet.rotation_speed_;
  float d = planet.distance_;
  glm::fvec3 color = planet.color_;

  //transform model matrix accoding to planet attributes
  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(r*glfwGetTime()), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*d});
  model_matrix = glm::scale(model_matrix, glm::fvec3{s, s, s});

  //upload matrices to current shader
  glUseProgram(m_shaders.at(shaderName).handle);
  glUniformMatrix4fv(m_shaders.at(shaderName).u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at(shaderName).u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));

//do not upload colors when using rainbow-shader
  if (shaderName == "planet_tex"){
    upload_texture(planet);
  } else if(shaderName != "rainbow"){
    //upload_camera_position();
    upload_color(color);
  }
}

void ApplicationSolar::upload_sun_transforms(planet const& sun) const{
  //render and upload planets
  float s = sun.size_;
  float r = sun.rotation_speed_;
  float d = sun.distance_;
  glm::fvec3 color = sun.color_;

  //transform model matrix accoding to planet attributes
  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(r*glfwGetTime()), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*d});
  model_matrix = glm::scale(model_matrix, glm::fvec3{s, s, s});

  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));

  glActiveTexture(GL_TEXTURE0 + sun.tex_);
  glBindTexture(GL_TEXTURE_2D, sun.tex_obj_.handle);

  int color_sampler_location = glGetUniformLocation(m_shaders.at("sun").handle, "ColorTex");
  glUseProgram(m_shaders.at("sun").handle);
  glUniform1i(color_sampler_location, sun.tex_);
}

void ApplicationSolar::upload_orbit_transforms(planet const& planet) const{
  float d = planet.distance_;

  //transform model matrix according to planet attributes
  glm::fmat4 model_matrix = glm::scale(glm::fmat4{}, glm::fvec3{d, d, d});

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

}

void ApplicationSolar::upload_orbit_transforms(moon const& moon) const{
  //get moons planet
  auto iter = planets.find(moon.planet_);
  if(iter == planets.end()){
    return;
  }
  planet planet = iter->second;


  float s = moon.size_;
  float r = moon.rotation_speed_;
  float d = moon.distance_;

  float p_r = planet.rotation_speed_;
  float p_d = planet.distance_;

  //transform model matrix according to moon attributes
  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(p_r*glfwGetTime()), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*p_d});
  model_matrix = glm::scale(model_matrix, glm::fvec3{d, d, d});

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));


}

//upload planet colors to blinn-phong and cel shader
void ApplicationSolar::upload_color(glm::fvec3 const& color) const {
  glUseProgram(m_shaders.at("planet").handle);
  glUniform3fv(m_shaders.at("planet").u_locs.at("matAmbient"),
                     1, glm::value_ptr(color));
  glUniform3fv(m_shaders.at("planet").u_locs.at("matDiffuse"),
                     1, glm::value_ptr(color));

  glUseProgram(m_shaders.at("planet_comic").handle);
  glUniform3fv(m_shaders.at("planet_comic").u_locs.at("matColor"),
                     1, glm::value_ptr(color));
  //bind current shader
  glUseProgram(m_shaders.at(shaderName).handle);

}

void ApplicationSolar::upload_texture(planet const& planet) const{
  glActiveTexture(GL_TEXTURE0 + planet.tex_);
  glBindTexture(GL_TEXTURE_2D, planet.tex_obj_.handle);

  int color_sampler_location = glGetUniformLocation(m_shaders.at("planet_tex").handle, "ColorTex");
  glUseProgram(m_shaders.at("planet_tex").handle);
  glUniform1i(color_sampler_location, planet.tex_);
}


void ApplicationSolar::updateProjection() {
  // upload matrix to gpu
  glUniform("ProjectionMatrix", m_view_projection);
}

// update uniform locations
void ApplicationSolar::uploadUniforms() {
  updateUniformLocations();

  updateView();
  updateProjection();
}


void ApplicationSolar::glUniform(std::string mat_name, glm::fmat4 mat){
  

  //bind shader, update matrix
  glUseProgram(m_shaders.at("stars").handle);
  glUniformMatrix4fv(m_shaders.at("stars").u_locs.at(mat_name),
                     1, GL_FALSE, glm::value_ptr(mat));

  //bind shader, update matrix
  glUseProgram(m_shaders.at("rainbow").handle);
  glUniformMatrix4fv(m_shaders.at("rainbow").u_locs.at(mat_name),
                     1, GL_FALSE, glm::value_ptr(mat));

  //bind shader, update matrix
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at(mat_name),
                     1, GL_FALSE, glm::value_ptr(mat));

  glUseProgram(m_shaders.at("planet_tex").handle);
  glUniformMatrix4fv(m_shaders.at("planet_tex").u_locs.at(mat_name),
                     1, GL_FALSE, glm::value_ptr(mat));

  //bind shader, update matrix
  glUseProgram(m_shaders.at("planet_comic").handle);
  glUniformMatrix4fv(m_shaders.at("planet_comic").u_locs.at(mat_name),
                     1, GL_FALSE, glm::value_ptr(mat));

  //bind shader, update matrix
  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at(mat_name),
                     1, GL_FALSE, glm::value_ptr(mat));

  //bind shader, update matrix
  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at(mat_name),
                     1, GL_FALSE, glm::value_ptr(mat));
}

// handle key input
void ApplicationSolar::keyCallback(int key, int scancode, int action, int mods) {
  //shifts camera position to front, negative z
  if (key == GLFW_KEY_W && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.1f});
    updateView();
  }
  //shifts camera position backwards, positive z
  else if (key == GLFW_KEY_S && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.1f});
    updateView();
  }
  //shifts camera position left, negative x
   else if (key == GLFW_KEY_A && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{-0.1f, 0.0f, 0.0f});
    updateView();
  }
  //shifts camera position right, positive x
   else if (key == GLFW_KEY_D && (action == GLFW_PRESS || GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.1f, 0.0f, 0.0f});
    updateView();
  }
  //change shader
    else if (key == GLFW_KEY_1 && (action == GLFW_PRESS)) {
      shaderName = "planet";
  }
  //change shader
    else if (key == GLFW_KEY_2 && (action == GLFW_PRESS)) {
      shaderName = "planet_comic";
  }
  //change shader
    else if (key == GLFW_KEY_3 && (action == GLFW_PRESS)) {
      shaderName = "rainbow";
  }
  //change shader
    else if (key == GLFW_KEY_4 && (action == GLFW_PRESS)) {
      shaderName = "planet_tex";
  }
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  // mouse handling
  float x = (float)pos_y;
  float y = (float)pos_x;
  //rotate camera view according to mouse movement
  m_view_transform = glm::rotate(m_view_transform, -0.01f, glm::fvec3{x, y, 0.0f});
  updateView();
}

// load shader programs
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("rainbow", shader_program{m_resource_path + "shaders/simple.vert",
                                           m_resource_path + "shaders/simple.frag"});
  // request uniform locations for shader program
  m_shaders.at("rainbow").u_locs["NormalMatrix"] = -1;
  m_shaders.at("rainbow").u_locs["ModelMatrix"] = -1;
  m_shaders.at("rainbow").u_locs["ViewMatrix"] = -1;
  m_shaders.at("rainbow").u_locs["ProjectionMatrix"] = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////

// store shader program objects in container
  m_shaders.emplace("sun", shader_program{m_resource_path + "shaders/simple_sun.vert",
                                           m_resource_path + "shaders/simple_sun.frag"});
  // request uniform locations for shader program
  m_shaders.at("sun").u_locs["NormalMatrix"] = -1;
  m_shaders.at("sun").u_locs["ModelMatrix"] = -1;
  m_shaders.at("sun").u_locs["ViewMatrix"] = -1;
  m_shaders.at("sun").u_locs["ProjectionMatrix"] = -1;
   m_shaders.at("sun").u_locs["ColorTex"] = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////


  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/blinn_phong_planet.vert",
                                           m_resource_path + "shaders/blinn_phong_planet.frag"});

  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["matAmbient"] = -1;
  m_shaders.at("planet").u_locs["matDiffuse"] = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////

  m_shaders.emplace("planet_tex", shader_program{m_resource_path + "shaders/blinn_phong_planet_tex.vert",
                                           m_resource_path + "shaders/blinn_phong_planet_tex.frag"});

  m_shaders.at("planet_tex").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet_tex").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet_tex").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet_tex").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet_tex").u_locs["ColorTex"] = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////


  m_shaders.emplace("planet_comic", shader_program{m_resource_path + "shaders/cel_shading_planet.vert",
                                           m_resource_path + "shaders/cel_shading_planet.frag"});

  m_shaders.at("planet_comic").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet_comic").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet_comic").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet_comic").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet_comic").u_locs["matColor"] = -1;


////////////////////////////////////////////////////////////////////////////////////////////////////

  m_shaders.emplace("stars", shader_program{m_resource_path + "shaders/simple_stars.vert",
                                           m_resource_path + "shaders/simple_stars.frag"});
  // request uniform locations for shader program
  m_shaders.at("stars").u_locs["ViewMatrix"] = -1;
  m_shaders.at("stars").u_locs["ProjectionMatrix"] = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////

  m_shaders.emplace("orbit", shader_program{m_resource_path + "shaders/simple_orbit.vert",
                                           m_resource_path + "shaders/simple_orbit.frag"});
  // request uniform locations for shader program
  m_shaders.at("orbit").u_locs["ModelMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ViewMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ProjectionMatrix"] = -1;

}

// load models
void ApplicationSolar::initializeGeometry() {
  //load planet in model
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", (model::NORMAL | model::TEXCOORD));
  //load stars in model
  model star_model = model{stars, (model::NORMAL | model::POSITION), {1}};
  //load orbit in model
  model orbit_model = model{orbit, (model::POSITION), {1}};

  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);  

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);
  // activate third attribute on gpu
  glEnableVertexAttribArray(2);
  // third attribute is 3 floats with no offset & stride
  glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);

  
  // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

   

  // store type of primitive to draw
  planet_object.draw_mode = GL_TRIANGLES;
  // transfer number of indices to model object 
  planet_object.num_elements = GLsizei(planet_model.indices.size());


  // generate vertex array object
  glGenVertexArrays(1, &star_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(star_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &star_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * star_model.data.size(), star_model.data.data(), GL_STATIC_DRAW);  

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, star_model.vertex_bytes, star_model.offsets[model::NORMAL]);

  // store type of primitive to draw
  star_object.draw_mode = GL_POINTS;
  // transfer number of indices to model object 
  star_object.num_elements = GLsizei(star_model.data.size()/6);

  // generate vertex array object
  glGenVertexArrays(1, &orbit_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(orbit_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &orbit_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, orbit_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * orbit_model.data.size(), orbit_model.data.data(), GL_STATIC_DRAW);  

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, orbit_model.vertex_bytes, orbit_model.offsets[model::POSITION]);

  // store type of primitive to draw
  orbit_object.draw_mode = GL_LINE_LOOP;
  // transfer number of indices to model object 
  orbit_object.num_elements = GLsizei(orbit_model.data.size()/3);

  glBindVertexArray(0); 

}

//fill vector with random numbers for x stars
void ApplicationSolar::addStars(unsigned int x){
  
  for(int i = 0; i < 6*x; ++i){
    stars.push_back(static_cast <float>(rand()%200)-100);
  }

}

//initialize orbit and fill vector so that it is a circle
void ApplicationSolar::initializeOrbit(){
  for(int i = 0; i < 359; ++i){
    orbit.push_back(cos((i*M_PI)/180));
    orbit.push_back(0.0f);
    orbit.push_back(-sin((i*M_PI)/180));
  }
//pushback the first coordinates so that the circle closes
  orbit.push_back(1.0f);
  orbit.push_back(0.0f);
  orbit.push_back(-0.0f);

}

void ApplicationSolar::initializeTextures(){
  for(auto& i : planets){
    std::string p_name = i.first;
    int tex_number = i.second.tex_;

    std::string path = "textures/" + p_name + ".png";
    std::cout << path << std::endl;

    pixel_data texture = texture_loader::file(m_resource_path + path);
    textures.push_back(texture);

    glActiveTexture(GL_TEXTURE0 + tex_number);
    glGenTextures(1, &i.second.tex_obj_.handle);
    glBindTexture(GL_TEXTURE_2D, i.second.tex_obj_.handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture.width, texture.height, 0, texture.channels, texture.channel_type, texture.ptr());
  }
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);

  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteBuffers(1, &star_object.element_BO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);

  glDeleteBuffers(1, &orbit_object.vertex_BO);
  glDeleteBuffers(1, &orbit_object.element_BO);
  glDeleteVertexArrays(1, &orbit_object.vertex_AO);
}

// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<ApplicationSolar>(argc, argv);
}