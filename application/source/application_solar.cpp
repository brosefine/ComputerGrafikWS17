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

const glm::fvec3 sun            {0.5f, 0.98039f, 0.75f};
const glm::fvec3 merkur         {0.69412f, 0.67843f, 0.67843f};
const glm::fvec3 venus          {0.89020f, 0.73333f, 0.46275};
const glm::fvec3 earth          {0.41961f, 0.57647f, 0.83922f};
const glm::fvec3 mars           {0.75686f, 0.26667f, 0.05490f};
const glm::fvec3 jupiter        {0.80784f, 0.64706f, 0.53725f};
const glm::fvec3 saturn         {0.76471f, 0.57255f, 0.30980f};
const glm::fvec3 uranus         {0.38431f, 0.68235f, 0.90588f};
const glm::fvec3 neptune        {0.23922f, 0.36863f, 0.97647f};
const glm::fvec3 grey           {0.5f, 0.5f, 0.5f};

glm::fmat4 rotation {};

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,star_object{}
 ,orbit_object{}
 ,planets{}
 ,moons{}
 ,stars{}
 ,orbit{}
{
  //add planets and moon
  planets.insert(std::pair<std::string, planet>("sun", {1.0f, 0.0f, 0.0f, sun, false}));
  planets.insert(std::pair<std::string, planet>("mercury", {0.25f, 0.02f, 3.0f, merkur, true}));
  planets.insert(std::pair<std::string, planet>("venus", {0.25f, 0.074f, 5.0f, venus, true}));
  planets.insert(std::pair<std::string, planet>("earth", {0.25f, 0.063f, 7.0f, earth, true}));
  planets.insert(std::pair<std::string, planet>("mars", {0.25f, 0.051f, 9.0f, mars, true}));
  planets.insert(std::pair<std::string, planet>("jupiter", {0.5f, 0.027f, 11.0f, jupiter, false}));
  planets.insert(std::pair<std::string, planet>("saturn", {0.5f, 0.020f, 13.0f, saturn, false}));
  planets.insert(std::pair<std::string, planet>("uranus", {0.3f, 0.014f, 15.0f, uranus, false}));
  planets.insert(std::pair<std::string, planet>("neptune", {0.3f, 0.011f, 17.0f, neptune, false}));

  moons.insert(std::pair<std::string, moon>("moon", {0.04f, 2.0f, 1.0f, grey, true, "earth"}));
  //sky is planet sphere
  //add stars
  addStars(1500);
  //add orbits
  initializeOrbit();
  initializeSquad();
  initializeFramebufferHandles();
  initializeFramebuffer();
  initializeUniformBuffers();
  //add textures
  initializeTextures();
  initializeGeometry();
  initializeShaderPrograms();
  
}


void ApplicationSolar::render() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_object.handle);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //render Sky before everything else
  renderSky();

  // bind shader to upload uniforms
  glBindVertexArray(star_object.vertex_AO);
  //draw stars
  glUseProgram(m_shaders.at("stars").handle);
  glDrawArrays(star_object.draw_mode, NULL, star_object.num_elements);  
  //iterate through planets and draw them
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

  std::string temp = shaderName;

  glBindFramebuffer(GL_FRAMEBUFFER, light_framebuffer_object.handle);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glClearColor(0.5f, 0.5f, 0.5f, 0.0f);  

   for(auto& i : planets){
    //render planet
    //upload_orbit_transforms(i.second);

    //glBindVertexArray(orbit_object.vertex_AO);
    //glDrawArrays(orbit_object.draw_mode, NULL, orbit_object.num_elements);

    if(i.first == "sun"){
      shaderName = "light";
    } else {    
      shaderName = "dark";
    }

    upload_planet_transforms(i.second);
    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }

  for(auto& i : moons){
    //render planet
    //upload_orbit_transforms(i.second);

    //glBindVertexArray(orbit_object.vertex_AO);
    //glDrawArrays(orbit_object.draw_mode, NULL, orbit_object.num_elements); 

    upload_moon_transforms(i.second);

    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }

  shaderName = temp;

  glBindVertexArray(0);
  renderSquad();
}

//render skysphere
void ApplicationSolar::renderSky() const {
  //disable depth test, so skybox is behind everything else
  glDepthMask(GL_FALSE);

  //upload matrices to sky shader
  glUseProgram(m_shaders.at("sky").handle);
  glUniformMatrix4fv(m_shaders.at("sky").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(rotation));

  //bind texture to shader
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, sky_texture_object.handle);

  int color_sampler_location = glGetUniformLocation(m_shaders.at("sky").handle, "ColorTex");
  glUseProgram(m_shaders.at("sky").handle);
  glUniform1i(color_sampler_location, 0);
  // bind the VAO to draw
  glBindVertexArray(planet_object.vertex_AO);

  // draw bound vertex array using bound shader
  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  //enable depth test again
  glDepthMask(1); 
}

//render screen quad
void ApplicationSolar::renderSquad() const {
  //calculate sun position on screen
  glm::fmat4 m = m_cameraBuffer.ProjectionMatrix * m_cameraBuffer.ViewMatrix * model_matrix_sun;
  glm::fvec4 lightPos =  m * glm::fvec4{0.0f, 0.0f, 0.0f, 1.0f};
  lightPos = lightPos/lightPos.z;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glUseProgram(m_shaders.at("squad").handle);
  //bind texture to shader
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_texture_object.handle);
  glUniform1i(m_shaders.at("squad").u_locs.at("ColorTex"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, light_texture_object.handle);
  glUniform1i(m_shaders.at("squad").u_locs.at("LightTex"), 1);

  glUniform4fv(m_shaders.at("squad").u_locs.at("lightPosition"), 1, glm::value_ptr(lightPos));
  // bind the VAO to draw
  // bind shader to upload uniforms
  glBindVertexArray(squad_object.vertex_AO);
  //draw stars
  glDrawArrays(squad_object.draw_mode, NULL, squad_object.num_elements);  
}

//update view matrix to all shaders
void ApplicationSolar::updateView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  m_cameraBuffer.ViewMatrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUniform();
}

//transform moon sphere and upload to shaders
void ApplicationSolar::upload_moon_transforms(moon const& moon) const{
  //get moons planet
  auto iter = planets.find(moon.planet_name_);
  if(iter == planets.end()){
    return;
  }
  planet planet = iter->second;

  //render and upload planets
  float s = moon.size_;
  float r = moon.rotation_speed_;
  float d = moon.distance_;
  glm::fvec3 color = moon.color_;

  float p_r = planet.rotation_speed_;
  float p_d = planet.distance_;

  //transform model matrix according to moon and planet attributes
  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(p_r*glfwGetTime()), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*p_d});
  model_matrix = glm::rotate(model_matrix, float(r*glfwGetTime()), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f*d});
  model_matrix = glm::scale(model_matrix, glm::fvec3{s, s, s});
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);

  if(moon.has_Normals_ && shaderName == "planet_tex")
  {//upload matrices to current shader
    glUseProgram(m_shaders.at("planet_normal_tex").handle);
    glUniformMatrix4fv(m_shaders.at("planet_normal_tex").u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));
  
    // extra matrix for normal transformation to keep them orthogonal to surface
    glUniformMatrix4fv(m_shaders.at("planet_normal_tex").u_locs.at("NormalMatrix"),
                       1, GL_FALSE, glm::value_ptr(normal_matrix));
  } else if (shaderName == "dark" || shaderName == "light"){
    glUseProgram(m_shaders.at(shaderName).handle);
    glUniformMatrix4fv(m_shaders.at(shaderName).u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));
  } else {
    //upload matrices to current shader
    glUseProgram(m_shaders.at(shaderName).handle);
    glUniformMatrix4fv(m_shaders.at(shaderName).u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));

    // extra matrix for normal transformation to keep them orthogonal to surface
    glUniformMatrix4fv(m_shaders.at(shaderName).u_locs.at("NormalMatrix"),
                       1, GL_FALSE, glm::value_ptr(normal_matrix));
  }


  //upload either texture or color
 if(shaderName != "dark" && shaderName != "light"){
      upload_texture(moon);
  }
  
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
  model_matrix = glm::rotate(model_matrix, float(glfwGetTime()), glm::fvec3{0.0f, 1.0f, 0.0f});
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);

  //upload matrices to normal shader if normal map exists
  if(planet.has_Normals_ && shaderName == "planet_tex")
  {//upload matrices to current shader
    glUseProgram(m_shaders.at("planet_normal_tex").handle);
    glUniformMatrix4fv(m_shaders.at("planet_normal_tex").u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));
  
    // extra matrix for normal transformation to keep them orthogonal to surface
    glUniformMatrix4fv(m_shaders.at("planet_normal_tex").u_locs.at("NormalMatrix"),
                       1, GL_FALSE, glm::value_ptr(normal_matrix));
  } else if (shaderName == "dark" || shaderName == "light"){
    glUseProgram(m_shaders.at(shaderName).handle);
    glUniformMatrix4fv(m_shaders.at(shaderName).u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));
  } else {
    //upload matrices to current shader
    glUseProgram(m_shaders.at(shaderName).handle);
    glUniformMatrix4fv(m_shaders.at(shaderName).u_locs.at("ModelMatrix"),
                       1, GL_FALSE, glm::value_ptr(model_matrix));

    // extra matrix for normal transformation to keep them orthogonal to surface
    glUniformMatrix4fv(m_shaders.at(shaderName).u_locs.at("NormalMatrix"),
                       1, GL_FALSE, glm::value_ptr(normal_matrix));
  }


  //upload either texture or color
  if(shaderName != "dark" && shaderName != "light"){
      upload_texture(planet);
  }
  
}

//transform sun sphere and upload to shaders
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
  model_matrix = glm::rotate(model_matrix, float(0.1f * glfwGetTime()), glm::fvec3{0.0f, 1.0f, 0.0f});

  model_matrix_sun = model_matrix;

  //upload matrices to sun shader
  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

  //bind texture to shader
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, sun.tex_objs_[0].handle);

  int color_sampler_location = glGetUniformLocation(m_shaders.at("sun").handle, "ColorTex");
  glUseProgram(m_shaders.at("sun").handle);
  glUniform1i(color_sampler_location, 0);
}

//transform orbit object to fit for planet
void ApplicationSolar::upload_orbit_transforms(planet const& planet) const{
  float d = planet.distance_;

  //transform model matrix according to planet attributes
  glm::fmat4 model_matrix = glm::scale(glm::fmat4{}, glm::fvec3{d, d, d});

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

}
//transform orbit object to fit for moon
void ApplicationSolar::upload_orbit_transforms(moon const& moon) const{
  //get moons planet
  auto iter = planets.find(moon.planet_name_);
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
  //upload model matrix to orbit shader
  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));

}

//upload planet colors to blinn-phong and cel shader
void ApplicationSolar::upload_color(glm::fvec3 const& color) const {
  glUseProgram(m_shaders.at("planet_comic").handle);
  glUniform3fv(m_shaders.at("planet_comic").u_locs.at("matColor"),
                     1, glm::value_ptr(color));
  //bind current shader
  glUseProgram(m_shaders.at(shaderName).handle);

}

//upload textures for given planet
void ApplicationSolar::upload_texture(planet const& planet) const{
  //if planet has a normal map, upload it to normal-texture shader
  if(planet.has_Normals_&& shaderName == "planet_tex"){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planet.tex_objs_[0].handle);

    int color_sampler_location = glGetUniformLocation(m_shaders.at("planet_normal_tex").handle, "ColorTex");
    glUseProgram(m_shaders.at("planet_normal_tex").handle);
    glUniform1i(color_sampler_location,0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, planet.tex_objs_[1].handle);

    color_sampler_location = glGetUniformLocation(m_shaders.at("planet_normal_tex").handle, "NormalTex");
    glUseProgram(m_shaders.at("planet_normal_tex").handle);
    glUniform1i(color_sampler_location, 1);
  } else if (shaderName == "planet_tex") {
  //if not, use ordinairy shader without normal map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planet.tex_objs_[0].handle);

    int color_sampler_location = glGetUniformLocation(m_shaders.at("planet_tex").handle, "ColorTex");
    glUseProgram(m_shaders.at("planet_tex").handle);
    glUniform1i(color_sampler_location, 0);
  }
  else{
    //if not, use ordinairy shader without normal map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planet.tex_objs_[0].handle);

    int color_sampler_location = glGetUniformLocation(m_shaders.at("planet_comic").handle, "ColorTex");
    glUseProgram(m_shaders.at("planet_comic").handle);
    glUniform1i(color_sampler_location, 0);
  }
}

//upload projection matrix to all shaders
void ApplicationSolar::updateProjection() {
  // upload matrix to gpu
  glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer_object.handle);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, GLsizei(m_width), GLsizei(m_height));

  glBindTexture(GL_TEXTURE_2D, m_texture_object.handle);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GLsizei(m_width), GLsizei(m_height), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  
  m_cameraBuffer.ProjectionMatrix = m_view_projection;
  glUniform();
}

// update uniform locations
void ApplicationSolar::uploadUniforms() {
  updateUniformLocations();

  
  std::vector<GLuint> prog_handles;
  prog_handles.push_back(m_shaders.at("stars").handle);
  prog_handles.push_back(m_shaders.at("planet_tex").handle);
  prog_handles.push_back(m_shaders.at("planet_normal_tex").handle);
  prog_handles.push_back(m_shaders.at("planet_comic").handle);
  prog_handles.push_back(m_shaders.at("orbit").handle);
  prog_handles.push_back(m_shaders.at("sun").handle);
  prog_handles.push_back(m_shaders.at("sky").handle);
  prog_handles.push_back(m_shaders.at("light").handle);
  prog_handles.push_back(m_shaders.at("dark").handle);


  for(auto i : prog_handles){
    GLuint location = glGetUniformBlockIndex(i, "CameraBlock");
    glUniformBlockBinding(i, location, 1);
  }

  updateView();
  updateProjection();
}

//upload given matrix to all shaders
void ApplicationSolar::glUniform(){  

  glBindBuffer(GL_UNIFORM_BUFFER, ubo_camera.handle);
  void* buffer_ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
  std::memcpy(buffer_ptr, &m_cameraBuffer, sizeof(glm::fmat4)*2);
  glUnmapBuffer(GL_UNIFORM_BUFFER);
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
      shaderName = "planet_tex";
  }
  //change shader
    else if (key == GLFW_KEY_2 && (action == GLFW_PRESS)) {
      shaderName = "planet_comic";
  }

    //change mode
    else if (key == GLFW_KEY_6 && (action == GLFW_PRESS)) {
      m_scatter = !m_scatter;

      glUseProgram(m_shaders.at("squad").handle);
      glUniform1i(m_shaders.at("squad").u_locs.at("scatter"), m_scatter);
  }
    //change mode
    else if (key == GLFW_KEY_7 && (action == GLFW_PRESS)) {
      m_greyscale = !m_greyscale;

      glUseProgram(m_shaders.at("squad").handle);
      glUniform1i(m_shaders.at("squad").u_locs.at("greyscale"), m_greyscale);
  }
    //change mode
    else if (key == GLFW_KEY_8 && (action == GLFW_PRESS)) {
      m_mirrored_h = !m_mirrored_h;

      glUseProgram(m_shaders.at("squad").handle);
      glUniform1i(m_shaders.at("squad").u_locs.at("mirrored_h"), m_mirrored_h);
  }//change mode
    else if (key == GLFW_KEY_9 && (action == GLFW_PRESS)) {
      m_mirrored_v = !m_mirrored_v;

      glUseProgram(m_shaders.at("squad").handle);
      glUniform1i(m_shaders.at("squad").u_locs.at("mirrored_v"), m_mirrored_v);
  }//change mode
    else if (key == GLFW_KEY_0 && (action == GLFW_PRESS)) {
      m_blur = !m_blur;

      glUseProgram(m_shaders.at("squad").handle);
      glUniform1i(m_shaders.at("squad").u_locs.at("blur"), m_blur);
  }

}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  // mouse handling
  //float x = (float)pos_y;
  //float y = (float)pos_x;
  //rotate camera view according to mouse movement
  m_view_transform = glm::rotate(m_view_transform, -0.01f, glm::fvec3{pos_y, pos_x, 0.0f});
  rotation = glm::rotate(rotation, 0.01f, glm::fvec3{pos_y, pos_x, 0.0f});
  updateView();
}

// load shader programs
void ApplicationSolar::initializeShaderPrograms() {

  // store shader program objects in container
  m_shaders.emplace("sun", shader_program{m_resource_path + "shaders/simple_sun.vert",
                                           m_resource_path + "shaders/simple_sun.frag"});
  // request uniform locations for shader program
  m_shaders.at("sun").u_locs["ModelMatrix"] = -1;
  m_shaders.at("sun").u_locs["CameraBlock"] = -1;
  m_shaders.at("sun").u_locs["ColorTex"] = -1;

  ////////////////////////////////////////////////////////////////////////////////////////////////////

   // store shader program objects in container
  m_shaders.emplace("sky", shader_program{m_resource_path + "shaders/sky.vert",
                                           m_resource_path + "shaders/sky.frag"});
  // request uniform locations for shader program
  m_shaders.at("sky").u_locs["ModelMatrix"] = -1;
  m_shaders.at("sky").u_locs["CameraBlock"] = -1;
  m_shaders.at("sky").u_locs["ColorTex"] = -1;

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  m_shaders.emplace("planet_tex", shader_program{m_resource_path + "shaders/blinn_phong_planet_tex.vert",
                                           m_resource_path + "shaders/blinn_phong_planet_tex.frag"});

  m_shaders.at("planet_tex").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet_tex").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet_tex").u_locs["CameraBlock"] = -1;
  m_shaders.at("planet_tex").u_locs["ColorTex"] = -1;

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  m_shaders.emplace("planet_normal_tex", shader_program{m_resource_path + "shaders/blinn_phong_planet_normal_tex.vert",
                                           m_resource_path + "shaders/blinn_phong_planet_normal_tex.frag"});

  m_shaders.at("planet_normal_tex").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet_normal_tex").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet_normal_tex").u_locs["CameraBlock"] = -1;
  m_shaders.at("planet_normal_tex").u_locs["ColorTex"] = -1;
  m_shaders.at("planet_normal_tex").u_locs["NormalTex"] = -1;

  ////////////////////////////////////////////////////////////////////////////////////////////////////


  m_shaders.emplace("planet_comic", shader_program{m_resource_path + "shaders/cel_shading_planet.vert",
                                           m_resource_path + "shaders/cel_shading_planet.frag"});

  m_shaders.at("planet_comic").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet_comic").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet_comic").u_locs["CameraBlock"] = -1;
  m_shaders.at("planet_comic").u_locs["ColorTex"] = -1;


  ////////////////////////////////////////////////////////////////////////////////////////////////////

  m_shaders.emplace("stars", shader_program{m_resource_path + "shaders/simple_stars.vert",
                                           m_resource_path + "shaders/simple_stars.frag"});
  // request uniform locations for shader program
  m_shaders.at("stars").u_locs["CameraBlock"] = -1;

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  m_shaders.emplace("orbit", shader_program{m_resource_path + "shaders/simple_orbit.vert",
                                           m_resource_path + "shaders/simple_orbit.frag"});
  // request uniform locations for shader program
  m_shaders.at("orbit").u_locs["ModelMatrix"] = -1;
  m_shaders.at("orbit").u_locs["CameraBlock"] = -1;

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  m_shaders.emplace("light", shader_program{m_resource_path + "shaders/yang.vert",
                                           m_resource_path + "shaders/yang.frag"});
  // request uniform locations for shader program
  m_shaders.at("light").u_locs["ModelMatrix"] = -1;
  m_shaders.at("light").u_locs["CameraBlock"] = -1;

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  m_shaders.emplace("dark", shader_program{m_resource_path + "shaders/yin.vert",
                                           m_resource_path + "shaders/yin.frag"});
  // request uniform locations for shader program
  m_shaders.at("dark").u_locs["ModelMatrix"] = -1;
  m_shaders.at("dark").u_locs["CameraBlock"] = -1;

  ////////////////////////////////////////////////////////////////////////////////////////////////////

   // store shader program objects in container
  m_shaders.emplace("squad", shader_program{m_resource_path + "shaders/simple_squad.vert",
                                           m_resource_path + "shaders/simple_squad.frag"});
  // request uniform locations for shader program
  m_shaders.at("squad").u_locs["ColorTex"] = -1;
  m_shaders.at("squad").u_locs["LightTex"] = -1;
  m_shaders.at("squad").u_locs["lightPosition"] = -1;
  m_shaders.at("squad").u_locs["greyscale"] = -1;
  m_shaders.at("squad").u_locs["mirrored_v"] = -1;
  m_shaders.at("squad").u_locs["mirrored_h"] = -1;
  m_shaders.at("squad").u_locs["blur"] = -1;
  m_shaders.at("squad").u_locs["scatter"] = -1;

}

// load models
void ApplicationSolar::initializeGeometry() {
  //load planet in model
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", (model::NORMAL | model::TEXCOORD | model::TANGENT));
  //load stars in model
  model star_model = model{stars, (model::NORMAL | model::POSITION), {1}};
  //load orbit in model
  model orbit_model = model{orbit, (model::POSITION), {1}};
  //load stars in model
  model squad_model = model{squad, (model::TEXCOORD | model::POSITION), {1}};
  ////////////////////////////////////////////////////////////////////////////////////////////////////

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
      // activate third attribute on gpu
    glEnableVertexAttribArray(3);
    // third attribute is 3 floats with no offset & stride
    glVertexAttribPointer(3, model::TANGENT.components, model::TANGENT.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TANGENT]);

    
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

  ////////////////////////////////////////////////////////////////////////////////////////////////////

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

  ////////////////////////////////////////////////////////////////////////////////////////////////////

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

  ////////////////////////////////////////////////////////////////////////////////////////////////////

    // generate vertex array object
    glGenVertexArrays(1, &squad_object.vertex_AO);
    // bind the array for attaching buffers
    glBindVertexArray(squad_object.vertex_AO);

    // generate generic buffer
    glGenBuffers(1, &squad_object.vertex_BO);
    // bind this as an vertex array buffer containing all attributes
    glBindBuffer(GL_ARRAY_BUFFER, squad_object.vertex_BO);
    // configure currently bound array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * squad_model.data.size(), squad_model.data.data(), GL_STATIC_DRAW);  

    // activate first attribute on gpu
    glEnableVertexAttribArray(0);
    // first attribute is 3 floats with no offset & stride
    glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, squad_model.vertex_bytes, squad_model.offsets[model::POSITION]);
    // activate second attribute on gpu
    glEnableVertexAttribArray(1);
    // second attribute is 3 floats with no offset & stride
    glVertexAttribPointer(1, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, squad_model.vertex_bytes, squad_model.offsets[model::TEXCOORD]);

    // store type of primitive to draw
    squad_object.draw_mode = GL_TRIANGLE_STRIP;
    // transfer number of indices to model object 
    squad_object.num_elements = GLsizei(squad_model.data.size()/5);

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  glBindVertexArray(0); 
}

void ApplicationSolar::initializeFramebufferHandles(){
  glGenRenderbuffers(1, &m_renderbuffer_object.handle);
  glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer_object.handle);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, GLsizei(1920u), GLsizei(1080u));

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &m_texture_object.handle);
  glBindTexture(GL_TEXTURE_2D, m_texture_object.handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GLsizei(1920u), GLsizei(1080u), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  glGenRenderbuffers(1, &light_renderbuffer_object.handle);
  glBindRenderbuffer(GL_RENDERBUFFER, light_renderbuffer_object.handle);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, GLsizei(1920u), GLsizei(1080u));

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &light_texture_object.handle);
  glBindTexture(GL_TEXTURE_2D, light_texture_object.handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GLsizei(1920u), GLsizei(1080u), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

void ApplicationSolar::initializeFramebuffer(){
  glGenFramebuffers(1, &m_framebuffer_object.handle);
  glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_object.handle); 
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 
                        m_texture_object.handle, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                        GL_RENDERBUFFER, m_renderbuffer_object.handle);

  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT1};
  glDrawBuffers(1, draw_buffers);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE) {std::cout << "ooooops" << std::endl;}

  //Framebuffer for God Rays
  glGenFramebuffers(1, &light_framebuffer_object.handle);
  glBindFramebuffer(GL_FRAMEBUFFER, light_framebuffer_object.handle); 
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 
                        light_texture_object.handle, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                        GL_RENDERBUFFER, light_renderbuffer_object.handle);

  GLenum light_draw_buffers[1] = {GL_COLOR_ATTACHMENT1};
  glDrawBuffers(1, light_draw_buffers);

  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE) {std::cout << "ooooops" << std::endl;}
}

void ApplicationSolar::initializeUniformBuffers(){
  glGenBuffers(1, &ubo_camera.handle);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_camera.handle);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::fmat4)*2, nullptr, GL_DYNAMIC_DRAW);
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

void ApplicationSolar::initializeSquad(){
  //v1 coordinates
  squad.push_back(-1.0f);
  squad.push_back(-1.0f);
  squad.push_back(0.0f);
  //v1 tex coordinates
  squad.push_back(0.0f);
  squad.push_back(0.0f);
  ////////////////////////////
  //v2 coordinates
  squad.push_back(1.0f);
  squad.push_back(-1.0f);
  squad.push_back(0.0f);
  //v2 tex coordinates
  squad.push_back(1.0f);
  squad.push_back(0.0f);
  ////////////////////////////
  //v4 coordinates
  squad.push_back(-1.0f);
  squad.push_back(1.0f);
  squad.push_back(0.0f);
  //v4 tex coordinates
  squad.push_back(0.0f);
  squad.push_back(1.0f);
  ////////////////////////////
  //v3 coordinates
  squad.push_back(1.0f);
  squad.push_back(1.0f);
  squad.push_back(0.0f);
  //v3 tex coordinates
  squad.push_back(1.0f);
  squad.push_back(1.0f);
}

//load all textures
void ApplicationSolar::initializeTextures(){
  //planet textures + sun
  for(auto& i : planets){
    //get planet name
    std::string p_name = i.first;
    //create file path
    std::string path = "textures/" + p_name;
    //load texture file from path
    pixel_data texture = texture_loader::file(m_resource_path + path + ".png");
    //new texture_object for current planet
    texture_object tex;

    //bind texture to texture_object
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture.width, texture.height, 0, texture.channels, texture.channel_type, texture.ptr());
    
    m_textures.push_back(tex.handle);
    i.second.tex_objs_.push_back(tex);
   //if current planet has a normal map, load it too
    if(i.second.has_Normals_){
      pixel_data n_texture = texture_loader::file(m_resource_path + path + "_normal.png");
      texture_object norm_tex;
      //new texture_object for current planet
      //bind normal map to texture_object
      glActiveTexture(GL_TEXTURE1);
      glGenTextures(1, &norm_tex.handle);
      glBindTexture(GL_TEXTURE_2D, norm_tex.handle);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, n_texture.width, n_texture.height, 0, n_texture.channels, n_texture.channel_type, n_texture.ptr());
      m_textures.push_back(norm_tex.handle);
      i.second.tex_objs_.push_back(norm_tex);
    }
  }

  //moon textures
  for(auto& i : moons){
    //get moon name
    std::string p_name = i.first;
    //create file path
    std::string path = "textures/" + p_name;
    //load texture from file
    pixel_data texture = texture_loader::file(m_resource_path + path + ".png");
    //new texture object for current moon
    texture_object tex;
    //bind texture to texture_objec
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture.width, texture.height, 0, texture.channels, texture.channel_type, texture.ptr());

    m_textures.push_back(tex.handle);
    i.second.tex_objs_.push_back(tex);
    if(i.second.has_Normals_){
      pixel_data n_texture = texture_loader::file(m_resource_path + path + "_normal.png");
      //new texture_object for current planet
      texture_object norm_tex;
      //bind normal map to texture_object
      glActiveTexture(GL_TEXTURE1);
      glGenTextures(1, &norm_tex.handle);
      glBindTexture(GL_TEXTURE_2D, norm_tex.handle);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, n_texture.width, n_texture.height, 0, n_texture.channels, n_texture.channel_type, n_texture.ptr());
    
      m_textures.push_back(norm_tex.handle);
      i.second.tex_objs_.push_back(norm_tex);
    }
  }

  //skysphere texture
  pixel_data texture = texture_loader::file(m_resource_path + "textures/sky.png");
  //bind sky texture to sky_texture_object, which is later given to skysphere
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &sky_texture_object.handle);
  glBindTexture(GL_TEXTURE_2D, sky_texture_object.handle);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture.width, texture.height, 0, texture.channels, texture.channel_type, texture.ptr());
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

  glDeleteBuffers(1, &squad_object.vertex_BO);
  glDeleteBuffers(1, &squad_object.element_BO);
  glDeleteVertexArrays(1, &squad_object.vertex_AO);

  glDeleteTextures(m_textures.size(), m_textures.data());
  glDeleteTextures(1, &m_texture_object.handle);
  glDeleteTextures(1, &sky_texture_object.handle);
  glDeleteTextures(1, &light_texture_object.handle);

  glDeleteFramebuffers(1, &m_framebuffer_object.handle);
  glDeleteFramebuffers(1, &light_framebuffer_object.handle);

  glDeleteRenderbuffers(1, &m_renderbuffer_object.handle);
  glDeleteRenderbuffers(1, &light_renderbuffer_object.handle);

  glDeleteBuffers(1, &ubo_camera.handle);

}

// exe entry point
int main(int argc, char* argv[]) {
  Launcher::run<ApplicationSolar>(argc, argv);
}