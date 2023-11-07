#include "./setup.hpp"
#include "glm/trigonometric.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "quaternion.hpp"

#include "config.h"

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
  glm::vec3 initTranslater(0);
  // glm::quat initRotate = glm::quat(glm::radians(glm::vec3(0, 90, 0))) *
  //                        glm::quat(glm::radians(glm::vec3(-90, 0, 0)));
  lvk::quaternion initRotate = lvk::from_euler_angles(glm::radians(glm::vec3(0.0f, 90.0f, 0.0f)))*
                         lvk::from_euler_angles(glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f)));

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

  // render loop
  // -----------
  gui.update([&]() {
    // per-frame time logic
    // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // accept and process all keyboard and mouse input
    // ----------------------------
    processInput(gui.window);

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

    glm::mat4 R = glm::transpose(initRotate.to_mat4());
    // translate: convert initTranslater to glm::mat4
    glm::mat4 T = glm::translate(glm::mat4(1.0f), initTranslater);
    // scale: convert modelScaler to glm::mat4
    glm::mat4 S = glm::scale(glm::mat4(1.0f), modelScaler);
    // we don's need any shear in our experiment

    // render
    // render depth map first
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(lmat.position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    depthShader.use();
    depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    depthShader.setMat4("model", T*R*S);
    ourModel.Draw(depthShader);
    T = glm::translate(glm::mat4(1.0f), initTranslater+glm::vec3(4.0f, 0.0f, 2.0f));
    depthShader.setMat4("model", T*R*S);
    ourModel.Draw(depthShader);
    T = glm::translate(glm::mat4(1.0f), initTranslater);

    // render the object with shadow map
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // load all value we need into shader
    auto setmodelShader = [&](dym::rdt::Shader &s, dym::rdt::Mesh &m) {
      s.use();
      // rotate first, translate latter, scale whenever
      s.setMat4("model", glm::mat4(1.f)*T*R*S);
      s.setVec3("viewPos", camera.Position);
      s.setMaterial("material", mat);
      s.setLightMaterial("light", lmat);
      s.setTexture("skybox", skybox.texture);
      s.setFloat("skylightIntensity", skylightIntensity);
      s.setTexture("depthMap", depthMap);
      bindOtherTexture(s);
      s.setMat4("lightSpaceMatrix", lightSpaceMatrix);
      s.setFloat("gamma", gamma);
      s.setVec3("F0", F0);

      s.setBool("existHeigTex", m.textures.size() == 4 || setReflect);
    };

    ourModel.Draw([&](dym::rdt::Mesh &m) -> dym::rdt::Shader & {
      setmodelShader(modelShader, m);
      return modelShader;
    });
    T = glm::translate(glm::mat4(1.0f), initTranslater+glm::vec3(4.0f, 0.0f, 2.0f));
    ourModel.Draw([&](dym::rdt::Mesh &m) -> dym::rdt::Shader & {
      setmodelShader(modelShader, m);
      return modelShader;
    });

    // load light value and draw light object
    lightShader.use();
    lightShader.setMat4("model", glm::mat4(1.f));
    lightShader.setVec3("lightPos", lmat.position);
    lightShader.setVec3("lightColor", lmat.ambient);
    // draw
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