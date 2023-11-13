#include "./setup.hpp"
#include "glm/trigonometric.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "quaternion.hpp"

#include "config.h"

void draw_plane(std::vector<glm::vec3> &v, glm::vec3 N, dym::rdt::Shader &shader) {
  GLfloat *vertices = new GLfloat[v.size()*6];

  // fill in vertices
  for (unsigned int i = 0; i < v.size(); ++i) {
    vertices[i*6] = v[i].x;
    vertices[i*6+1] = v[i].y;
    vertices[i*6+2] = v[i].z;
    vertices[i*6+3] = N.x;
    vertices[i*6+4] = N.y;
    vertices[i*6+5] = N.z;
  }

  unsigned int VAO, VBO;

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*v.size(), vertices, GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // normal attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  shader.use();

  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLE_FAN, 0, v.size());

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  delete vertices;
}

glm::vec3 collider_bottom_P(0.0f, -20.0f, 0.0f);
glm::vec3 collider_bottom_N(0.0f, 1.0f, 0.0f);

glm::vec3 collider_A_P(-20.0f, 0.0f, 0.0f);
glm::vec3 collider_A_N(1.0f, 0.0f, 0.0f);

glm::vec3 collider_B_P(20.0f, 00.0f, 0.0f);
glm::vec3 collider_B_N(-1.0f, 0.0f, 0.0f);

glm::vec3 collider_C_P(0.0f, 0.0f, 20.0f);
glm::vec3 collider_C_N(0.0f, 0.0f, -1.0f);

glm::vec3 collider_D_P(0.0f, 0.0f, -20.0f);
glm::vec3 collider_D_N(0.0f, 0.0f, 1.0f);

inline float norm(glm::vec3 &vec) {
  return std::sqrt(vec.x*vec.x+vec.y*vec.y+vec.z*vec.z);
}

class RigidBody {
private:
  dym::rdt::Model model;
  glm::mat3 scale;
  // reference inertia
  glm::mat3 I_ref;
  // the velocity
  glm::vec3 v;
  glm::vec3 w;
  // for velocity decay
  float linear_decay = 0.999f;
  float angular_decay = 0.98f;
  // for collision
  float restitution = 0.5f;
  // friction coefficient
  float friction = 0.2f;

  // gravity acceleration
  glm::vec3 gravity;
  float mass;
  // position
  glm::vec3 x;
  // quaternion for rotation
  lvk::quaternion q;

  glm::mat3 cross_product_mat(glm::vec3 &v) {
    return glm::mat3(0.0f, -v.z,  v.y,
                      v.z, 0.0f, -v.x,
                     -v.y,  v.x, 0.0f);
  }

  void model_vertice_iteration(std::function<void(dym::rdt::Vertex)> &&f) {
    for (auto &&mesh : model.meshes) {
      for (auto &&vert : mesh.vertices) {
        f(vert);
      }
    }
  }
  void compute_inertia_mat() {
    unsigned int V = 0;
    for (auto &&mesh : model.meshes) {
      V += mesh.vertices.size();
    }
    float m_i = mass/V;
    I_ref = glm::mat3(0.0f);
    model_vertice_iteration([&](dym::rdt::Vertex vert) {
      glm::vec3 r_i = vert.Position;
      float rr = r_i.x*r_i.x+r_i.y*r_i.y+r_i.z*r_i.z;
      float x = r_i.x;
      float y = r_i.y;
      float z = r_i.z;
      I_ref += m_i*glm::mat3(
        rr-x*x,-x*y,-x*z,
        -y*z,rr-y*y,-y*z,
        -z*x,-z*y,rr-z*z
      );
    });
  }
  glm::mat3 inertia_inverse() {
    glm::mat3 R = q.to_mat3();
    return glm::inverse(R*I_ref*glm::transpose(R));
  }
  // collision would cause the change of velocity
  void impulse_collision(glm::vec3 P, glm::vec3 N) {
    unsigned int collision_count = 0;
    glm::vec3 collision_vertex(0.0f);
    model_vertice_iteration([&](dym::rdt::Vertex vert) {
      // check if one vertex is inside the collision plane
      glm::vec3 r_i = scale*vert.Position; // position of local space
      glm::vec3 Rr_i = q*r_i; // rotate first
      glm::vec3 x_i = Rr_i+x; // then translate
      if (glm::dot((x_i-P), N) < 0.0f) {
        // if the object still moves into the plane
        glm::vec3 v_i = v+glm::cross(w, Rr_i);
        if (glm::dot(v_i, N) < 0.0f) {
          collision_count++;
          collision_vertex += r_i;
        }
      }
    });
    if (collision_count == 0) return;
    // the average collision vertex
    glm::vec3 r_collision = 1.0f/collision_count*collision_vertex;
    // rotated collotion vertex
    glm::vec3 Rr_collision = q*r_collision;
    glm::vec3 v_collision = v+glm::cross(w, Rr_collision);

    glm::vec3 v_N = glm::dot(v_collision, N)*N;
    glm::vec3 v_T = v_collision-v_N;
    glm::vec3 v_T_new = std::max(0.0f, 
    1.0f-friction*(1.0f+restitution)*norm(v_N)/norm(v_T))*v_T;
    glm::vec3 v_N_new = -restitution*v_N;
    glm::vec3 v_new = v_T_new+v_N_new;
    glm::mat3 Rr_i_star = cross_product_mat(Rr_collision);
    glm::mat3 I_inverse = inertia_inverse();
    glm::mat3 K = 1.0f/mass*glm::mat3(1.0f)-Rr_i_star*I_inverse*Rr_i_star;
    glm::vec3 j = glm::inverse(K)*(v_new-v_collision);

    v += 1.0f/mass*j;
    w += I_inverse*glm::cross(Rr_collision, j);
  }
public:
  RigidBody(
    dym::rdt::Model &_model,
    glm::vec3 _gravity,
    float _mass,
    lvk::quaternion _initR,
    glm::vec3 _initT,
    glm::vec3 _initS
  ) {
    v = glm::vec3(0.0f);
    w = glm::vec3(0.0f);
    set_model(_model);
    set_gravity(_gravity);
    set_mass(_mass);
    set_init_rotation(_initR);
    set_init_translation(_initT);
    glm::mat3 S = glm::mat3(glm::scale(glm::mat4(1.0f), _initS));
    set_scale(S);
    compute_inertia_mat();
  }
  ~RigidBody() {}
  // called in each rendering loop
  // update the matrix for translation and rotation
  void update(float dt) {
    v += dt*gravity;
    v *= linear_decay;
    w *= angular_decay;
    // perform collision test
    impulse_collision(collider_bottom_P, collider_bottom_N);
    // impulse_collision(collider_A_P, collider_A_N);
    impulse_collision(collider_B_P, collider_B_N);
    // impulse_collision(collider_C_P, collider_C_N);
    // impulse_collision(collider_D_P, collider_D_N);

    // update position and quaternion rotation
    x += dt*v;
    glm::vec3 dw = 0.5f*dt*w;
    lvk::quaternion qw(0.0f, dw.x, dw.y, dw.z);
    q = q + qw*q;
  }
  glm::mat4 get_R() {
    return glm::transpose(q.to_mat4());
  }
  glm::mat4 get_T() {
    return glm::translate(glm::mat4(1.0f), x);
  }
  glm::vec3 get_X() {return x;}
  void set_model(dym::rdt::Model &m) {
    model = m;
  }
  void set_gravity(glm::vec3 &g) {gravity = g;}
  void set_mass(float m) {mass = m;}
  void set_init_rotation(lvk::quaternion &quat) {q = quat;}
  void set_init_translation(glm::vec3 &trans) {x = trans;}
  void set_scale(glm::mat3 &scale) {this->scale = scale;}

  void add_velocity(glm::vec3 &velocity) {
    v += velocity;
  }
  void add_rotation(glm::vec3 &wVelocity) {
    w += wVelocity;
  }
  void adjust_parameters(
    float restitution = 0.5f,
    float friction = 0.2f,
    float linear_decay = 0.999f,
    float angular_decay = 0.98f
  ) {
    this->restitution = restitution;
    this->friction = friction;
    this->linear_decay = linear_decay;
    this->angular_decay = angular_decay;
  }
};

const GLuint SHADOW_WIDTH = 1024;
const GLuint SHADOW_HEIGHT = 1024;

int main() {
  // init gui and imgui
  dym::GUI gui(SCR_WIDTH, SCR_HEIGHT, framebuffer_size_callback, mouse_callback,
               scroll_callback);
  UseImGui myimgui;
  myimgui.Init(gui.window, "#version 460 core");
  float gamma = 1.2f;

  // tell stb_image.h to flip loaded texture's on the y-axis (before loading
  // model).
  stbi_set_flip_vertically_on_load(true);
  bool setReflect = false;

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);

  // build and compile shaders
  // -------------------------------
  // load models' shader
  dym::rdt::Shader modelShader(SOURCE_DIR "/shaders/model_render.vs",
                               SOURCE_DIR "/shaders/model_render.fs");
  // load luminous objects' shader
  dym::rdt::Shader lightShader(SOURCE_DIR  "/shaders/lightbox.vs",
                               SOURCE_DIR  "/shaders/lightbox.fs");
  // load sky box shader
  dym::rdt::Shader skyboxShader(SOURCE_DIR  "/shaders/skybox.vs", SOURCE_DIR  "/shaders/skybox.fs");

  // shader for depth map
  dym::rdt::Shader depthShader(SOURCE_DIR "/shaders/shadow/depthShader.vs", SOURCE_DIR "/shaders/shadow/depthShader.fs");

  // shader for plane rendering
  dym::rdt::Shader planeShader(SOURCE_DIR "/shaders/plane.vs", SOURCE_DIR "/shaders/plane.fs");

  // load models
  // -----------
  // // 1. backpack
  // dym::rdt::Model ourModel(SOURCE_DIR "/resources/objects/backpack/backpack.obj");
  // modelShader.setTexture("texture_height1", 0);
  // auto bindOtherTexture = [&](dym::rdt::Shader &s) { return; };
  // glm::vec3 modelScaler(1.);
  // glm::vec3 initTranslater(0.);
  // // glm::quat initRotate(1, glm::vec3(0, 0, 0));
  // lvk::quaternion initRotate(1.0f, 0.0f, 0.0f, 0.0f);

  // // 2. nanosuit
  // dym::rdt::Model ourModel(SOURCE_DIR "/resources/objects/nanosuit/nanosuit.obj");
  // auto bindOtherTexture = [&](dym::rdt::Shader &s) { return; };
  // glm::vec3 modelScaler(1.);
  // glm::vec3 initTranslater(0, -10, 0);
  // // glm::quat initRotate(1, glm::vec3(0, 0, 0));
  // lvk::quaternion initRotate(1.0f, 0.0f, 0.0f, 0.0f);

  // 3. cerberus gun
  dym::rdt::Model ourModel(
      SOURCE_DIR "/resources/objects/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
  // load other textures
  dym::rdt::Texture normalTex(
      SOURCE_DIR "/resources/objects/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga",
      dym::rdt::TexType.normal);
  dym::rdt::Texture specularTex(
      SOURCE_DIR "/resources/objects/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga",
      dym::rdt::TexType.specular);
  dym::rdt::Texture reflectTex(SOURCE_DIR "/resources/objects/Cerberus_by_Andrew_Maximov/"
                               "Textures/Cerberus_R.tga",
                               dym::rdt::TexType.height);
  auto bindOtherTexture = [&](dym::rdt::Shader &s) {
    s.setTexture("texture_normal1", normalTex.id);
    s.setTexture("texture_specular1", specularTex.id);
    s.setTexture("texture_height1", reflectTex.id);
  };
  setReflect = true;
  glm::vec3 modelScaler(0.05f);
  glm::vec3 initTranslater(0.0f, 0.0f, 0.0f);
  // glm::quat initRotate = glm::quat(glm::radians(glm::vec3(0, 90, 0))) *
  //                        glm::quat(glm::radians(glm::vec3(-90, 0, 0)));
  lvk::quaternion initRotate = lvk::from_euler_angles(glm::radians(glm::vec3(0.0f, 90.0f, 0.0f)))*
                         lvk::from_euler_angles(glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f)));

  lvk::quaternion midRotate1 = lvk::from_euler_angles(glm::radians(glm::vec3(20.0f, -120.0f, 90.0f)));
  lvk::quaternion midRotate2 = lvk::from_euler_angles(glm::radians(glm::vec3(120.0f, -10.0f, 0.0f)));

  // dym::rdt::Model ourModel(SOURCE_DIR "/resources/objects/cube.obj");
  // auto bindOtherTexture = [&](dym::rdt::Shader &s) {};

  // create framebuffer for depth mapping
  GLuint depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);

  // create shadow map
  GLuint depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
               SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // bind shadow map as depth map for depth buffer
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  // no need for color buffer
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  // restore to default frame buffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // moving light Cube
  // -----------------
  dym::rdo::Cube lightCube;

  // set Material parameter
  // ----------------------
  dym::rdt::Material mat({0.2, 0.2, 0.2}, {1.0, 1.0, 1.0}, {0.5, 0.5, 0.5},
                         32.);

  glm::vec3 F0(0.04f);
  dym::rdt::PoLightMaterial lmat({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0},
                                 {1.0, 1.0, 1.0}, 1.0, 0.045f, 0.0075f);

  // load skyBox
  // -----------------
  // SkyBox Texture
  std::vector<std::string> faces{"right.jpg",  "left.jpg",  "top.jpg",
                                 "bottom.jpg", "front.jpg", "back.jpg"};
  for (auto &face : faces)
    face = SOURCE_DIR "/resources/textures/skybox/" + face;
  // load skyBox
  dym::rdo::SkyBox skybox;
  skybox.loadCubeTexture(faces);
  float skylightIntensity = 1.0;

  // draw in wireframe
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // shared projection matrix for all shader
  // ---------------------------------------------
  dym::rdt::UniformBuffer proj_ubuffer(sizeof(glm::mat4) * 2, 0);
  proj_ubuffer.bindShader(modelShader, "Object");
  proj_ubuffer.bindShader(skyboxShader, "Object");
  proj_ubuffer.bindShader(lightShader, "Object");

  // time recorder
  // -------------
  int i = 0;

  camera.Position = {0, 0, 20};
  camera.MouseSensitivity = 0.1f;

  // set lightCube position and rotate
  // ---------------------------------------
  lmat.position = {10, 0, 0};
  // glm::quat lightR(glm::radians(glm::vec3(0, 2, 0)));
  lvk::quaternion lightR = lvk::from_euler_angles(glm::radians(glm::vec3(0.0f, 2.0f, 0.0f)));
  // NOTE: you can use qprint to check quaterion's value
  // qprint(lightR.w, lightR.x, lightR.y, lightR.z);

  // properties of the camera
  float zNear = 0.1f;
  float zFar = 100.0f;
  float aspect = SCR_WIDTH*1.0f/SCR_HEIGHT;

  // the interval for each rotation of the light source
  // unit in seconds
  const double rotation_interval = 0.05;
  double rotation_timer = 0;

  bool enableSkyLight = true;

  GLfloat near_plane = 0.01f, far_plane = 100.0f;

  float rotateState = 0.0f;
  bool rotateModel = false;

  RigidBody rb(ourModel, glm::vec3(0.0f, -9.8f, 0.0f), 10.0f, initRotate, initTranslater, modelScaler);

  glm::vec3 translator = initTranslater;

  float restitution = 0.5f;
  float friction = 0.2f;
  float linear_decay = 0.999f;
  float angular_decay = 0.98f;

  std::vector<glm::vec3> plane_vertex_a {
    glm::vec3(-20.0f, -20.0f, -20.0f),
    glm::vec3(20.0f, -20.0f, -20.0f),
    glm::vec3(20.0f, -20.0f, 20.0f),
    glm::vec3(-20.0f, -20.0f, 20.0f),
  };
  glm::vec3 plane_normal_a(0.0f, 1.0f, 0.0f);

  std::vector<glm::vec3> plane_vertex_b {
    glm::vec3(20.0f, -20.0f, -20.0f),
    glm::vec3(20.0f, 20.0f, -20.0f),
    glm::vec3(20.0f, 20.0f, 20.0f),
    glm::vec3(20.0f, -20.0f, 20.0f),
  };
  glm::vec3 plane_normal_b(-1.0f, 0.0f, 0.0f);

  float dt = 0.008f;
  // render loop
  // -----------
  gui.update([&]() {
    // per-frame time logic
    // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    rb.adjust_parameters(restitution, friction, linear_decay, angular_decay);

    // while (deltaTime > 0.0f) {
    //   rb.update(0.1f);
    //   deltaTime -= 0.1f;
    // }
    rb.update(dt);

    // accept and process all keyboard and mouse input
    // ----------------------------
    processInput(gui.window);

    glm::vec3 deltaVelocity(1.0f, 0.0f, 0.0f);
    glm::vec3 deltaRotation(2.0f, 0.0f, 0.0f);
    if (glfwGetKey(gui.window, GLFW_KEY_Q) == GLFW_PRESS) {
      rb.add_velocity(deltaVelocity);
    }
    if (glfwGetKey(gui.window, GLFW_KEY_R) == GLFW_PRESS) {
      rb.add_rotation(deltaRotation);
    }

    // mygui update
    myimgui.Update();
    // render imgui
    {
      ImGui::Begin("Settings");
      if constexpr (false)
        ImGui::ShowDemoWindow();
      ImGui::Text("Object's Material Settings");
      ImGui::SliderFloat("objmat.ambient.r", &(mat.ambient[0]), 0, 1,
                         "ambient.r = %.2f");
      ImGui::SliderFloat("objmat.ambient.g", &(mat.ambient[1]), 0, 1,
                         "ambient.g = %.2f");
      ImGui::SliderFloat("objmat.ambient.b", &(mat.ambient[2]), 0, 1,
                         "ambient.b = %.2f");
      ImGui::SliderFloat("objmat.diffuse.x", &(mat.diffuse[0]), 0, 1,
                         "diffuse.x = %.2f");
      ImGui::SliderFloat("objmat.diffuse.y", &(mat.diffuse[1]), 0, 1,
                         "diffuse.y = %.2f");
      ImGui::SliderFloat("objmat.diffuse.z", &(mat.diffuse[2]), 0, 1,
                         "diffuse.z = %.2f");
      ImGui::SliderFloat("objmat.specular.x", &(mat.specular[0]), 0, 1,
                         "specular.x = %.2f");
      ImGui::SliderFloat("objmat.specular.y", &(mat.specular[1]), 0, 1,
                         "specular.y = %.2f");
      ImGui::SliderFloat("objmat.specular.z", &(mat.specular[2]), 0, 1,
                         "specular.z = %.2f");
      ImGui::SliderFloat("F0.x", &(F0[0]), 0, 1, "F0.x = %.2f");
      ImGui::SliderFloat("F0.y", &(F0[1]), 0, 1, "F0.y = %.2f");
      ImGui::SliderFloat("F0.z", &(F0[2]), 0, 1, "F0.z = %.2f");
      ImGui::SliderFloat("shininess", &(mat.shininess), 0, 1, "shininess = %.2f");
      ImGui::SliderFloat("skylightIntensity", &(skylightIntensity), 0, 5,
                         "intensity = %.2f");
      ImGui::Checkbox("enable skylight", &enableSkyLight);
      ImGui::Checkbox("enable model rotation", &rotateModel);
      ImGui::Text("Light Settings");
      ImGui::SliderFloat("light.ambient.r", &(lmat.ambient[0]), 0, 1,
                         "ambient.r = %.2f");
      ImGui::SliderFloat("light.ambient.g", &(lmat.ambient[1]), 0, 1,
                         "ambient.g = %.2f");
      ImGui::SliderFloat("light.ambient.b", &(lmat.ambient[2]), 0, 1,
                         "ambient.b = %.2f");
      ImGui::SliderFloat("light.diffuse.x", &(lmat.diffuse[0]), 0, 1,
                         "diffuse.x = %.2f");
      ImGui::SliderFloat("light.diffuse.y", &(lmat.diffuse[1]), 0, 1,
                         "diffuse.y = %.2f");
      ImGui::SliderFloat("light.diffuse.z", &(lmat.diffuse[2]), 0, 1,
                         "diffuse.z = %.2f");
      ImGui::SliderFloat("light.specular.x", &(lmat.specular[0]), 0, 1,
                         "specular.x = %.2f");
      ImGui::SliderFloat("light.specular.y", &(lmat.specular[1]), 0, 1,
                         "specular.y = %.2f");
      ImGui::SliderFloat("light.specular.z", &(lmat.specular[2]), 0, 1,
                         "specular.z = %.2f");
      ImGui::SliderFloat("gamma", &(gamma), 1, 3, "gamma = %.2f");
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();

      ImGui::Begin("Model transformation");
      ImGui::SliderFloat("scale.x", &(modelScaler.x), 0, 1,
                         "scale.x = %.2f");
      ImGui::SliderFloat("scale.y", &(modelScaler.y), 0, 1,
                         "scale.y = %.2f");
      ImGui::SliderFloat("scale.z", &(modelScaler.z), 0, 1,
                         "scale.z = %.2f");
      ImGui::SliderFloat("translate.x", &(initTranslater.x), -10, 10,
                         "translate.x = %.2f");
      ImGui::SliderFloat("translate.y", &(initTranslater.y), -10, 10,
                         "translate.y = %.2f");
      ImGui::SliderFloat("translate.z", &(initTranslater.z), -10, 10,
                         "translate.z = %.2f");
      // ImGui::SliderFloat("rotate.x", &(modelScaler.x), 0, 1,
      //                    "rotate.x = %.2f");
      // ImGui::SliderFloat("rotate.y", &(modelScaler.y), 0, 1,
      //                    "rotate.y = %.2f");
      // ImGui::SliderFloat("rotate.z", &(modelScaler.z), 0, 1,
      //                    "rotate.z = %.2f");
      ImGui::End();

      ImGui::Begin("Camera settings");
      ImGui::SliderFloat("zNear", &(zNear), 0, 1,
                         "zNear = %.2f");
      ImGui::SliderFloat("zFar", &(zFar), 50, 100,
                         "zFar = %.1f");
      ImGui::SliderFloat("aspect", &(aspect), 0.5, 2,
                         "aspect = %.2f");
      ImGui::SliderFloat("fovy", &(camera.Zoom), 1.0f, 110.f,
                         "fovy = %.1f");
      ImGui::End();

      ImGui::Begin("Physics Parameters");
      ImGui::SliderFloat("restitution", &restitution, 0.0f, 1.0f, "restitution = %.1f");
      ImGui::SliderFloat("friction", &friction, 0.0f, 1.0f, "friction = %.1f");
      ImGui::SliderFloat("linear_decay", &linear_decay, 0.0f, 2.0f, "linear_decay = %.3f");
      ImGui::SliderFloat("angular_decay", &angular_decay, 0.0f, 2.0f, "angular_decay = %.3f");
      ImGui::SliderFloat("dt", &dt, 0.0f, 0.5f, "dt = %.4f");
      ImGui::End();
    }

    // use quat lightR to rotate lightbox
    if (currentFrame-rotation_timer >= rotation_interval) {
      lmat.position = lightR*lmat.position;
      rotation_timer = currentFrame;
    }

    // use glm to create projection and view matrix
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, zNear, zFar);
    glm::mat4 view = camera.GetViewMatrix();

    // load projection and view into shaders' uniformBuffer
    proj_ubuffer.use();
    proj_ubuffer.setMat4(0, projection);
    proj_ubuffer.setMat4(1, view);
    proj_ubuffer.close();

    glm::mat4 R;
    if (rotateModel) {
      // rotation matrix : initRotate -> midRotate1 -> midRotate2 -> initRotate
      if (rotateState <= 1.0f) {
        R = glm::transpose(lvk::slerp(initRotate, midRotate1, rotateState).to_mat4());
        rotateState += deltaTime;
      } else if (rotateState <= 2.0f) {
        R = glm::transpose(lvk::slerp(midRotate1, midRotate2, rotateState-1.0f).to_mat4());
        rotateState += deltaTime;
      } else if (rotateState <= 3.0f) {
        R = glm::transpose(lvk::slerp(midRotate2, initRotate, rotateState-2.0f).to_mat4());
        rotateState += deltaTime;
      } else rotateState = 0.0f;
    } else {
      // R = glm::transpose(initRotate.to_mat4());
      R = rb.get_R();
    }
    // translate: convert initTranslater to glm::mat4
    // glm::mat4 T = glm::translate(glm::mat4(1.0f), initTranslater);
    glm::mat4 T = rb.get_T();
    // scale: convert modelScaler to glm::mat4
    glm::mat4 S = glm::scale(glm::mat4(1.0f), modelScaler);
    // we don's need any shear in our experiment

    // render
    // render depth map first
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(lmat.position, rb.get_X(), glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), rb.get_X()-lmat.position)));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    depthShader.use();
    depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    depthShader.setMat4("model", T*R*S);
    ourModel.Draw(depthShader);
    // draw_plane(plane_vertex_a, plane_normal_a, depthShader);
    // draw_plane(plane_vertex_b, plane_normal_b, depthShader);

    // render the object with shadow map
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    ourModel.Draw([&](dym::rdt::Mesh &m) -> dym::rdt::Shader & {
      modelShader.use();
      modelShader.setMat4("model", glm::mat4(1.f)*T*R*S);
      modelShader.setVec3("viewPos", camera.Position);
      modelShader.setMaterial("material", mat);
      modelShader.setLightMaterial("light", lmat);
      modelShader.setTexture("skybox", skybox.texture);
      modelShader.setFloat("skylightIntensity", skylightIntensity);
      modelShader.setTexture("depthMap", depthMap);
      bindOtherTexture(modelShader);
      modelShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
      modelShader.setFloat("gamma", gamma);
      modelShader.setVec3("F0", F0);

      modelShader.setBool("existHeigTex", m.textures.size() == 4 || setReflect);
      return modelShader;
    });

    planeShader.use();
    planeShader.setMat4("view", view);
    planeShader.setMat4("projection", projection);
    planeShader.setLightMaterial("light", lmat);
    planeShader.setTexture("depthMap", depthMap);
    planeShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    planeShader.setVec3("PassInColor", glm::vec3(0.5f, 0.5f, 0.5f));
    draw_plane(plane_vertex_a, plane_normal_a, planeShader);
    planeShader.setVec3("PassInColor", glm::vec3(0.6f, 0.6f, 0.6f));
    draw_plane(plane_vertex_b, plane_normal_b, planeShader);

    // load light value and draw light object
    lightShader.use();
    lightShader.setMat4("model", glm::mat4(1.f));
    lightShader.setVec3("lightPos", lmat.position);
    lightShader.setVec3("lightColor", lmat.ambient);
    // draw
    lightCube.Draw(lightShader);
    lightShader.setVec3("lightPos", glm::vec3(0, -20, 0));
    lightShader.setVec3("lightColor", glm::vec3(1.0f, 0.0f, 0.0f));
    lightCube.Draw(lightShader);
    lightShader.setVec3("lightPos", glm::vec3(0, -40, 0));
    lightShader.setVec3("lightColor", glm::vec3(0.0f, 1.0f, 0.0f));
    lightCube.Draw(lightShader);

    // load skybox value and draw skybox
    skyboxShader.use();
    skyboxShader.setTexture("skybox", skybox.texture);
    skyboxShader.setVec3("offset", camera.Position);
    skyboxShader.setBool("enableSkyLight", enableSkyLight);
    // draw
    skybox.Draw(skyboxShader);

    // render gui
    // ----------
    myimgui.Render();
  });
  // close imgui
  myimgui.Shutdown();

  // after main end, GUI will be closed by ~GUI() automatically.
  return 0;
}