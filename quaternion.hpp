#ifndef MY_QUATERNION_HPP
#define MY_QUATERNION_HPP

#include <cmath>
#include <utility>
#include <iostream>

#include <glm/glm.hpp>

namespace lvk {

class quaternion {
public:
  float x, y, z, w, norm;
  quaternion() {}
  quaternion(float _w, float _x, float _y, float _z) : x(_x), y(_y), z(_z), w(_w) {
    norm = sqrt(x*x+y*y+z*z+w*w);
  }
  // construct form a rotation axis and an angle
  quaternion(float angle, glm::vec3 axis) {
    axis = glm::normalize(axis);
    w = std::cos(angle/2);
    x = axis.x*sin(angle/2);
    y = axis.y*sin(angle/2);
    z = axis.z*sin(angle/2);
    norm = sqrt(x*x+y*y+z*z+w*w);
  }
  quaternion(quaternion &&quat) {
    x = std::move(quat.x);
    y = std::move(quat.y);
    z = std::move(quat.z);
    w = std::move(quat.w);
    norm = std::move(quat.norm);
  }
  quaternion(quaternion &quat) {
    x = quat.x;
    y = quat.y;
    z = quat.z;
    w = quat.w;
    norm = quat.norm;
  }
  quaternion& operator=(quaternion &&quat) {
    x = std::move(quat.x);
    y = std::move(quat.y);
    z = std::move(quat.z);
    w = std::move(quat.w);
    norm = std::move(quat.norm);
    return *this;
  }
  quaternion& operator=(quaternion &quat) {
    x = quat.x;
    y = quat.y;
    z = quat.z;
    w = quat.w;
    norm = quat.norm;
    return *this;
  }

  void normalize() {
    if (norm == 0) return;
    x/=norm;y/=norm;z/=norm;w/=norm;norm=1.0f;
  }
  quaternion normalized() {
    if (norm == 0) return quaternion(0.0f, 0.0f, 0.0f, 0.0f);
    return quaternion(w/norm, x/norm, y/norm, z/norm);
  }
  quaternion conjugate() {return quaternion(w, -x, -y, -z);}
  quaternion inverse() {
    if (norm == 0) return quaternion(0.0f, 0.0f, 0.0f, 0.0f);
    return quaternion(w/norm, -x/norm, -y/norm, -z/norm);
  }

  void display() {printf("w:%f,x:%f,y:%f,z:%f", w, x, y, z);}

  glm::mat3 get_mat3() {
    if (norm = 0) return glm::mat3(1.0f);
    float a = w/norm;
    float b = x/norm;
    float c = y/norm;
    float d = z/norm;
    return glm::mat3(
      1-2*c*c-2*d*d, 2*b*c-2*a*d, 2*a*c+2*b*d,
      2*b*c+2*a*d, 1-2*b*b-2*d*d, 2*c*d-2*a*b,
      2*b*d-2*a*c, 2*a*b+2*c*d, 1-2*b*b-2*c*c
    );
  }
  glm::mat4 to_mat4() {
    if (norm == 0) return glm::mat4(1.0f);
    float a = w/norm;
    float b = x/norm;
    float c = y/norm;
    float d = z/norm;
    return glm::mat4(
      1-2*c*c-2*d*d, 2*b*c-2*a*d, 2*a*c+2*b*d, 0.0f,
      2*b*c+2*a*d, 1-2*b*b-2*d*d, 2*c*d-2*a*b, 0.0f,
      2*b*d-2*a*c, 2*a*b+2*c*d, 1-2*b*b-2*c*c, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    );
  }
};

// grabmann product
quaternion operator*(quaternion p, quaternion q) {
  glm::vec3 v(p.x, p.y, p.z);
  glm::vec3 u(q.x, q.y, q.z);
  float s = p.w;
  float t = q.w;
  glm::vec3 vec = s*u+t*v+glm::cross(v, u);
  return quaternion(s*t-glm::dot(v, u), vec.x, vec.y, vec.z);
}
quaternion operator+(quaternion p, quaternion q) {
  return quaternion(p.w+q.w, p.x+q.x, p.y+q.y, p.z+q.z);
}
quaternion operator-(quaternion p, quaternion q) {
  return quaternion(p.w-q.w, p.x-q.x, p.y-q.y, p.z-q.z);
}
quaternion operator-(quaternion q) {
  return quaternion(-q.w, -q.x, -q.y, -q.z);
}
glm::vec3 operator*(quaternion &q, glm::vec3 &vec) {
  quaternion v(0, vec.x, vec.y, vec.z);
  q.normalize();
  auto tmp = q*v*q.inverse();
  return glm::vec3(tmp.x, tmp.y, tmp.z);
}
quaternion operator*(float k, quaternion &q) {
  return quaternion(k*q.w, k*q.x, k*q.y, k*q.z);
}
quaternion lerp(quaternion q0, quaternion q1, const float t) {
  q0.normalize();
  q1.normalize();
  return (1.0f-t)*q0+t*q1;
}
quaternion slerp(quaternion q0, quaternion q1, const float t) {
  q0.normalize();
  q1.normalize();
  glm::vec3 v0(q0.x, q0.y, q0.z);
  glm::vec3 v1(q1.x, q1.y, q1.z);
  float dot_val = glm::dot(v0, v1);
  if (dot_val < 0) {
    dot_val = -dot_val;
    q0 = -q0;
  }
  float theta = std::acos(dot_val);
  return std::sin((1.0f-t)*theta)/std::sin(theta)*q0+std::sin(t*theta)/std::sin(theta)*q1;
}
quaternion from_euler_angles(float phi_x, float phi_y, float phi_z) {
  quaternion qx(phi_x, glm::vec3(1, 0, 0));
  quaternion qy(phi_y, glm::vec3(0, 1, 0));
  quaternion qz(phi_z, glm::vec3(0, 0, 1));
  return qz*qy*qx;
}
quaternion from_euler_angles(glm::vec3 euler_angles) {
  quaternion qx(euler_angles.x, glm::vec3(1, 0, 0));
  quaternion qy(euler_angles.y, glm::vec3(0, 1, 0));
  quaternion qz(euler_angles.z, glm::vec3(0, 0, 1));
  return qz*qy*qx;
}

};

#endif