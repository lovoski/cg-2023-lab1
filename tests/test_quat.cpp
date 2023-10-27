#include "quaternion.hpp"
#include <glm/gtc/quaternion.hpp>
#include <iostream>

using namespace std;

inline void print_glm_vec3(glm::vec3 &vec) {
  cout << vec.x << " " << vec.y << " " << vec.z << "\n";
}

void test_rotation() {
  glm::vec3 v(1.0f, 0.0f, 1.0f);

  glm::vec3 euler_anlges_1 = glm::radians(glm::vec3(0.0f, 180.0f, 0.0f));
  glm::vec3 euler_anlges_2 = glm::radians(glm::vec3(0.0f, 180.0f, 90.0f));
  glm::vec3 euler_anlges_3 = glm::radians(glm::vec3(90.0f, 180.0f, -120.0f));
  lvk::quaternion q_lvk_1 = lvk::from_euler_angles(euler_anlges_1);
  glm::quat q_glm_1(euler_anlges_1);

  lvk::quaternion q_lvk_2 = lvk::from_euler_angles(euler_anlges_2);
  glm::quat q_glm_2(euler_anlges_2);

  lvk::quaternion q_lvk_3 = lvk::from_euler_angles(euler_anlges_3);
  glm::quat q_glm_3(euler_anlges_3);

  cout << "rotation:\n";
  print_glm_vec3(q_lvk_1*v);
  print_glm_vec3(q_glm_1*v);
  print_glm_vec3(q_lvk_2*v);
  print_glm_vec3(q_glm_2*v);
  print_glm_vec3(q_lvk_3*v);
  print_glm_vec3(q_glm_3*v);

  glm::vec3 rotation_axis_1(1.0f, 0.0f, 0.0f);
  glm::vec3 rotation_axis_2 = glm::normalize(glm::vec3((1.0f, 1.0f, 2.0f)));
  glm::vec3 rotation_axis_3 = glm::normalize(glm::vec3((1.1f, 2.1f, 2.0f)));
  const float angle = glm::radians(123.0f);
  lvk::quaternion q_lvk_4(angle, rotation_axis_1);
  glm::quat q_glm_4 = glm::angleAxis(angle, rotation_axis_1);

  lvk::quaternion q_lvk_5(angle, rotation_axis_2);
  glm::quat q_glm_5 = glm::angleAxis(angle, rotation_axis_2);

  lvk::quaternion q_lvk_6(angle, rotation_axis_3);
  glm::quat q_glm_6 = glm::angleAxis(angle, rotation_axis_3);

  print_glm_vec3(q_lvk_4*v);
  print_glm_vec3(q_glm_4*v);
  print_glm_vec3(q_lvk_5*v);
  print_glm_vec3(q_glm_5*v);
  print_glm_vec3(q_lvk_6*v);
  print_glm_vec3(q_glm_6*v);
}

void test_interpolate(float t) {
  glm::vec3 euler_angles_1 = glm::radians(glm::vec3(-90.0f, 122.0f, 0.0f));
  glm::vec3 euler_angles_2 = glm::radians(glm::vec3(0.0f, 90.0f, 90.0f));
  lvk::quaternion q1_lvk = lvk::from_euler_angles(euler_angles_1);
  lvk::quaternion q2_lvk = lvk::from_euler_angles(euler_angles_2);
  glm::quat q1_glm(euler_angles_1);
  glm::quat q2_glm(euler_angles_2);
  auto qa_lvk = lvk::slerp(q1_lvk, q2_lvk, t).normalized();
  auto qa_glm = glm::normalize(glm::mix(q1_glm, q2_glm, t));

  cout << "interpolate:\n";
  print_glm_vec3(glm::vec3(qa_lvk.x, qa_lvk.y, qa_lvk.z));
  print_glm_vec3(glm::vec3(qa_glm.x, qa_glm.y, qa_glm.z));
}

int main() {
  test_rotation();
  test_interpolate(0.22f);
  return 0;
}