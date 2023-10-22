#pragma once

// gl,pic
#include "../HGL"
#include <string>

namespace dym {
namespace rdt {
class UniformBuffer {
public:
  unsigned int ubo, bufferSize, bindId;
  std::string name = "UniformBuffer";
  UniformBuffer(unsigned int bufferSize, unsigned int bindId)
      : bufferSize(bufferSize), bindId(bindId) {
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, bindId, ubo, 0, bufferSize);
    prefixOffset.push_back(0);
  }

private:
  std::vector<unsigned int> prefixOffset;

  inline unsigned int findIndex(const unsigned int &index,
                                const unsigned int objSize) {
    auto pfos = prefixOffset.size();
    if (index > pfos) {
      DYM_ERROR_cs(name, "Please check the index! (index:" +
                             std::to_string(index) + ")");
      return 0;
    }
    if (index == pfos)
      prefixOffset.push_back(prefixOffset[pfos - 1] + objSize);
    return prefixOffset[index];
  }

public:
  inline void use() { glBindBuffer(GL_UNIFORM_BUFFER, ubo); }
  inline void close() { glBindBuffer(GL_UNIFORM_BUFFER, 0); }
  void buildOffsetprefixTable(const std::vector<unsigned int> &offset) {
    prefixOffset.clear();
    prefixOffset.push_back(0);
    for (int i = 1; i < offset.size(); ++i)
      prefixOffset.push_back(prefixOffset[i - 1] + offset[i]);
  }

  void bindShader(Shader &shader, const std::string &blockName) {
    glUniformBlockBinding(shader.ID,
                          glGetUniformBlockIndex(shader.ID, blockName.c_str()),
                          bindId);
  }

  // ------------------------------------------------------------------------
  void setBool(const unsigned int &index, bool value) {
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(value)),
                    sizeof(value), &value);
  }
  // ------------------------------------------------------------------------
  void setInt(const unsigned int &index, int value) {
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(value)),
                    sizeof(value), &value);
  }
  // ------------------------------------------------------------------------
  void setFloat(const unsigned int &index, float value) {
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(value)),
                    sizeof(value), &value);
  }
  // // ------------------------------------------------------------------------
  // void setTexture(const unsigned int &index, const std::string &name,
  //                 int texIndex, unsigned int textureID) {
  //   glActiveTexture(GL_TEXTURE0 + texIndex);
  //   glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(textureID)),
  //                   sizeof(textureID), &textureID);
  //   glBindTexture(GL_TEXTURE_2D, textureID);
  // }
  // void setTexture(const unsigned int &index, const std::string &name,
  //                 unsigned int textureID) {
  //   auto texIndex = texRegFind(ubo + 114514, name);
  //   glActiveTexture(GL_TEXTURE0 + texIndex);
  //   glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(textureID)),
  //                   sizeof(textureID), &textureID);
  //   glBindTexture(GL_TEXTURE_2D, textureID);
  // }
  // ------------------------------------------------------------------------
  void setVec2(const unsigned int &index, glm::vec2 value) {
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(value)),
                    sizeof(value), glm::value_ptr(value));
  }
  void setVec2(const unsigned int &index, float x, float y) {
    glm::vec2 value{x, y};
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(value)),
                    sizeof(value), glm::value_ptr(value));
  }
  // ------------------------------------------------------------------------
  void setVec3(const unsigned int &index, glm::vec3 value) {
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(value)),
                    sizeof(value), glm::value_ptr(value));
  }
  void setVec3(const unsigned int &index, float x, float y, float z) {
    glm::vec3 value{x, y, z};
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(value)),
                    sizeof(value), glm::value_ptr(value));
  }
  // ------------------------------------------------------------------------
  void setVec4(const unsigned int &index, glm::vec4 value) {
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(value)),
                    sizeof(value), glm::value_ptr(value));
  }
  void setVec4(const unsigned int &index, float x, float y, float z, float w) {
    glm::vec4 value{x, y, z, w};
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(value)),
                    sizeof(value), glm::value_ptr(value));
  }
  // ------------------------------------------------------------------------
  void setMat2(const unsigned int &index, glm::mat2 mat) {
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(mat)),
                    sizeof(mat), glm::value_ptr(mat));
  }
  // ------------------------------------------------------------------------
  void setMat3(const unsigned int &index, glm::mat3 mat) {
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(mat)),
                    sizeof(mat), glm::value_ptr(mat));
  }
  // ------------------------------------------------------------------------
  void setMat4(const unsigned int &index, glm::mat4 mat) {
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(mat)),
                    sizeof(mat), glm::value_ptr(mat));
  }
  // ------------------------------------------------------------------------
  void setMaterial(const unsigned int &index, const Material &mat) {
    setVec3(index, mat.ambient);
    setVec3(index, mat.diffuse);
    setVec3(index, mat.specular);
    setFloat(index, mat.shininess);
  }
  void setLightMaterial(const unsigned int &index, const LightMaterial &mat) {
    setVec3(index, mat.position);
    setVec3(index + 1, mat.ambient);
    setVec3(index + 2, mat.diffuse);
    setVec3(index + 3, mat.specular);
  }
  void setLightMaterial(const unsigned int &index, const PaLightMaterial &mat) {
    setVec3(index, mat.direction);
    setVec3(index + 1, mat.ambient);
    setVec3(index + 2, mat.diffuse);
    setVec3(index + 3, mat.specular);
  }
  void setLightMaterial(const unsigned int &index, const PoLightMaterial &mat) {
    glBufferSubData(GL_UNIFORM_BUFFER, findIndex(index, sizeof(mat)),
                    sizeof(mat), &(mat));
    // setVec3(index, mat.position);
    // setVec3(index + 1, mat.ambient);
    // setVec3(index + 2, mat.diffuse);
    // setVec3(index + 3, mat.specular);
    // setFloat(index + 4, mat.constant);
    // setFloat(index + 5, mat.linear);
    // setFloat(index + 6, mat.quadratic);
  }
  void setLightMaterial(const unsigned int &index,
                        const SpotLightMaterial &mat) {
    setVec3(index, mat.position);
    setVec3(index + 1, mat.direction);
    setFloat(index + 2, mat.inCutOff);
    setFloat(index + 3, mat.outCutOff);
    setVec3(index + 4, mat.ambient);
    setVec3(index + 5, mat.diffuse);
    setVec3(index + 6, mat.specular);
  }
};
} // namespace rdt
} // namespace dym