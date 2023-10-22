/*
 * @Author: DyllanElliia
 * @Date: 2021-09-25 16:01:48
 * @LastEditTime: 2023-09-27 08:43:27
 * @LastEditors: DyllanElliia
 * @Description: Syntactic sugar!
 */
#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <ostream>
#include <sstream>
#include <vector>
#include <array>

namespace tType {
const std::string NONE("0"), BOLD("1"), DIM("2"), UNDERLINE("4"), BLINK("5"),
    INVERSE("7"), HIDDEN("8");
}
namespace tColor {
const std::string BLACK("30"), RED("31"), GREEN("32"), YELLOW("33"), BLUE("34"),
    MAGENTA("35"), CYAN("36");
}
template <typename T, typename... Ts>
void qp_ctrl(T v, Ts... vl) {
  std::string ctrl("\033[");
  ctrl += std::string(v);
  if constexpr (sizeof...(vl) > 0) {
    std::array cl = {std::string(vl)...};
    for (auto &c : cl) ctrl += ";" + c;
  }
  ctrl += "m";
  std::cout << ctrl;
}
void qp_ctrl() { std::cout << "\033[0m"; }

// Print the values no line break.
template <typename T, typename... Ts>
void qprint_nlb(T v, Ts... vl) {
  std::cout << v << " ";
  if constexpr (sizeof...(vl) > 0) {
    qprint_nlb(vl...);
    return;
  }
  // std::cout << std::endl;
}

// Print the values with line break.
template <typename T, typename... Ts>
void qprint(T v, Ts... vl) {
  std::cout << v << " ";
  if constexpr (sizeof...(vl) > 0) {
    qprint(vl...);
    return;
  }
  std::cout << std::endl;
}

inline void qprint() { printf("\n"); }

#include <chrono>
#include <ctime>
namespace dym {
class TimeLog {
 private:
  // double timep;
  std::chrono::steady_clock::time_point timep;
  std::vector<std::pair<float, float>> timeLogs;
  bool flag;

  // #if defined(USE_WIN)
  // #define ONE_SECOND 1000
  // #elif defined(USE_LINUX)
  // #define ONE_SECOND 1000000
  // #endif // USE_WIN

 public:
  auto getRecord(double scale = 1) {
    flag = false;
    auto end = std::chrono::steady_clock::now();
    auto tt =
        std::chrono::duration_cast<std::chrono::duration<float>>(end - timep);
    return tt.count() * scale;
  }
  void record(double scale = 1) {
    flag = false;
    auto end = std::chrono::steady_clock::now();
    auto tt =
        std::chrono::duration_cast<std::chrono::duration<float>>(end - timep);
    std::cout << "Run time: " << tt.count() << "s" << std::endl;
  }

  void reStart() { timep = std::chrono::steady_clock::now(); }

  void saveLog() {
    auto end = std::chrono::steady_clock::now();
    auto tt =
        std::chrono::duration_cast<std::chrono::duration<float>>(end - timep);
    timeLogs.push_back(std::make_pair(timeLogs.size(), tt.count()));
  }

  void saveLog(float tag) {
    auto end = std::chrono::steady_clock::now();
    auto tt =
        std::chrono::duration_cast<std::chrono::duration<float>>(end - timep);
    timeLogs.push_back(std::make_pair(tag, tt.count()));
  }

  TimeLog() {
    timep = std::chrono::steady_clock::now();
    flag = true;
  }

  ~TimeLog() {
    if (flag) {
      record();
    }
  }
};
}  // namespace dym

#define DYM_ERROR(errorString) __DYM_ERROR_CALL(errorString, __FILE__, __LINE__)

#define DYM_ERROR_cs(className, errorString)                          \
  __DYM_ERROR_CALL(std::string(className) + " Error: " + errorString, \
                   __FILE__, __LINE__)

void __DYM_ERROR_CALL(std::string err, const char *file, const int line) {
  qp_ctrl(tColor::RED, tType::BOLD, tType::UNDERLINE);
  qprint(err, "\n--- error in file <", file, ">, line", line, ".\n");
  qp_ctrl();
}

#define DYM_WARNING(str) __DYM_WARNING_CALL(str, __FILE__, __LINE__)
#define DYM_WARNING_cs(className, wString)                            \
  __DYM_WARNING_CALL(std::string(className) + " Warning: " + wString, \
                     __FILE__, __LINE__)

void __DYM_WARNING_CALL(std::string err, const char *file, const int line) {
  qp_ctrl(tColor::YELLOW, tType::BOLD, tType::UNDERLINE);
  qprint(err, "\n--- warning in file <", file, ">, line", line, ".\n");
  qp_ctrl();
}

#define _DYM_ASSERT_(bool_opt, outstr) \
  try {                                \
    if (bool_opt) throw outstr;        \
  } catch (const char *str) {          \
    DYM_ERROR(str);                    \
    exit(EXIT_FAILURE);                \
  }

typedef float lReal;
typedef double Real;
typedef int Reali;
typedef unsigned int uReali;

namespace dym {
const Real Pi = 3.1415926535897932385;
#define PI Pi
#define DYM_TEMPLATE_CHECK(T, TYPE) \
  template <typename T,             \
            std::enable_if_t<std::is_convertible<T, TYPE>::value, int> = 0>
#define _DYM_TEMPLATE_CHECK_(T, TYPE) DYM_TEMPLATE_CHECK(T, TYPE)
}  // namespace dym