#include "std/target_os.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if defined(OMIM_OS_WINDOWS)
  #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(OMIM_OS_LINUX)
  #define GLFW_EXPOSE_NATIVE_X11
#elif defined(OMIM_OS_MAC)
  #define GLFW_EXPOSE_NATIVE_COCOA
#else
  #error Unsupported plaform
#endif
#include <GLFW/glfw3native.h>

#include "map/framework.hpp"

#include "platform/platform.hpp"
#include "platform/settings.hpp"

#include "coding/reader.hpp"

#include "base/logging.hpp"
#include "base/macros.hpp"

#include <chrono>
#include <functional>
#include <sstream>

#include <gflags/gflags.h>

DEFINE_string(data_path, "", "Path to data directory.");
DEFINE_string(log_abort_level, base::ToString(base::GetDefaultLogAbortLevel()), "Log messages severity that causes termination.");
DEFINE_string(resources_path, "", "Path to resources directory.");
DEFINE_string(lang, "", "Device language.");

#if defined(PLATFORM_MAC)
void * createCocoaWindowView(GLFWwindow * window);
void updateContentScale(GLFWwindow * window, float scale);
#endif

namespace
{
bool ValidateLogAbortLevel(char const * flagname, std::string const & value)
{
  if (auto level = base::FromString(value); !level)
  {
    std::cerr << "Invalid value for --" << flagname << ": "<< value << ", must be one of: ";
    auto const & names = base::GetLogLevelNames();
    for (size_t i = 0; i < names.size(); ++i)
    {
      if (i != 0)
        std::cerr << ", ";
      std::cerr << names[i];
    }
    std::cerr << '\n';
    return false;
  }
  return true;
}

bool const g_logAbortLevelDummy = gflags::RegisterFlagValidator(&FLAGS_log_abort_level, &ValidateLogAbortLevel);

void errorCallback(int error, char const * description) 
{
  LOG(LERROR, ("GLFW (", error, "):", description));
}

std::function<void(int w, int h)> onResize;
std::function<void(double x, double y, int button, int action, int mods)> onMouseButton;
std::function<void(double x, double y)> onMouseMove;
std::function<void(double xOffset, double yOffset)> onScroll;
std::function<void(int key, int scancode, int action, int mods)> onKeyboardButton;
std::function<void(float xscale, float yscale)> onContentScale;
} // namespace

int main(int argc, char * argv[])
{
  // Our double parsing code (base/string_utils.hpp) needs dots as a floating point delimiters, not commas.
  // TODO: Refactor our doubles parsing code to use locale-independent delimiters.
  // For example, https://github.com/google/double-conversion can be used.
  // See http://dbaron.org/log/20121222-locale for more details.
  (void)::setenv("LC_NUMERIC", "C", 1);

  Platform & platform = GetPlatform();

  LOG(LINFO, ("Organic Maps: Developer Sandbox", platform.Version(), "detected CPU cores:", platform.CpuCores()));

  gflags::SetUsageMessage("Developer Sandbox.");
  gflags::SetVersionString(platform.Version());
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (!FLAGS_resources_path.empty())
    platform.SetResourceDir(FLAGS_resources_path);
  if (!FLAGS_data_path.empty())
    platform.SetWritableDirForTests(FLAGS_data_path);

  if (auto const logLevel = base::FromString(FLAGS_log_abort_level); logLevel)
    base::g_LogAbortLevel = *logLevel;
  else
    LOG(LCRITICAL, ("Invalid log level:", FLAGS_log_abort_level));

  // Init GLFW.
  glfwSetErrorCallback(errorCallback);
  if (!glfwInit()) {
    return -1;
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#if defined(OMIM_OS_WINDOWS)
  glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#endif
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
  auto monitor = glfwGetPrimaryMonitor();
  auto mode = glfwGetVideoMode(monitor);
  GLFWwindow * window = glfwCreateWindow(mode->width, mode->height,
                                         "Organic Maps: Developer Sandbox",
                                         nullptr, nullptr);
  int fbWidth = 0, fbHeight = 0;
  glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
  float xs = 1.0f, ys = 1.0f;
  glfwGetWindowContentScale(window, &xs, &ys);

  platform.SetupMeasurementSystem();

  bool outvalue;
  if (!settings::Get(settings::kDeveloperMode, outvalue))
    settings::Set(settings::kDeveloperMode, true);

  if (!FLAGS_lang.empty())
    (void)::setenv("LANGUAGE", FLAGS_lang.c_str(), 1);

  FrameworkParams frameworkParams;
  Framework framework(frameworkParams);

  // Process resizing.
  onResize = [&](int w, int h) {
    fbWidth = w;
    fbHeight = h;
    if (fbWidth > 0 && fbHeight > 0) {
      //TODO: app->resize(static_cast<uint32_t>(fbWidth), static_cast<uint32_t>(fbHeight));
    }
  };
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow * wnd, int w, int h) { onResize(w, h); });

  // Process change content scale.
  onContentScale = [&](float xscale, float yscale) {
    //TODO: app->updateContentScale(std::max(xscale, yscale));

    int w = 0, h = 0;
    glfwGetWindowSize(window, &w, &h);
#if defined(OMIM_OS_MAC)
    w *= xscale;
    h *= yscale;
#endif

    if (w != fbWidth || h != fbHeight) {
#if defined(OMIM_OS_MAC)
      updateContentScale(window, xscale);
#endif
      fbWidth = w;
      fbHeight = h;
      //TODO: app->resize(w, h);
    }
  };
  glfwSetWindowContentScaleCallback(window, [](GLFWwindow *, float xscale, float yscale) 
  {
    onContentScale(xscale, yscale);
  });

  // Handle mouse buttons.
  onMouseButton = [&](double x, double y, int button, int action, int mods) 
  {
    // TODO: app->mouseButton(glm::vec2(static_cast<float>(x), static_cast<float>(y)),
    //                   button,
    //                   action,
    //                   mods);
  };
  glfwSetMouseButtonCallback(window, [](GLFWwindow * wnd, int button, int action, int mods) 
  {
    // TODO: double x, y;
    // glfwGetCursorPos(wnd, &x, &y);

    // ImGuiMouseButton_ const imguiButton =
    //   (button == GLFW_MOUSE_BUTTON_LEFT)
    //     ? ImGuiMouseButton_Left
    //     : (button == GLFW_MOUSE_BUTTON_RIGHT ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);
    // ImGuiIO & io = ImGui::GetIO();
    // io.MousePos = ImVec2(static_cast<float>(x), static_cast<float>(y));
    // io.MouseDown[imguiButton] = action == GLFW_PRESS;
    // if (!io.WantCaptureMouse) {
    //   onMouseButton(x, y, button, action, mods);
    // }
  });

  // Handle mouse moving.
  onMouseMove = [&](double x, double y) 
  {
    //TODO: app->mouseMove(glm::vec2(static_cast<float>(x), static_cast<float>(y)));
  };
  glfwSetCursorPosCallback(window, [](GLFWwindow *, double x, double y) 
  {
    //TODO: ImGui::GetIO().MousePos = ImVec2(x, y);
    //onMouseMove(x, y);
  });

  // Handle scroll.
  onScroll = [&](double xOffset, double yOffset) 
  {
    //TODO: constexpr double kSensitivity = 0.01;
    //app->scroll(glm::vec2(static_cast<float>(xOffset * kSensitivity),
    //                      static_cast<float>(yOffset * kSensitivity)));
  };
  glfwSetScrollCallback(window, [](GLFWwindow *, double xoffset, double yoffset)
  {
    //TODO: ImGuiIO & io = ImGui::GetIO();
    //io.MouseWheel = static_cast<float>(yoffset);
    //io.MouseWheelH = static_cast<float>(xoffset);
    //if (!io.WantCaptureMouse) {
    //  onScroll(xoffset, yoffset);
    //}
  });

  // Keys.
  onKeyboardButton = [&](int key, int scancode, int action, int mods) 
  {
    //TODO: app->keyboardButton(key, scancode, action, mods);
  };
  glfwSetKeyCallback(window, [](GLFWwindow *, int key, int scancode, int action, int mods) 
  {
    //TODO: onKeyboardButton(key, scancode, action, mods);
  });

  // Main loop.
  auto lastTime = std::chrono::steady_clock::now();
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    auto const now = std::chrono::steady_clock::now();
    auto const duration = now - lastTime;
    auto const elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0;

    if (fbWidth > 0 && fbHeight > 0) {
      //app->render(elapsed);
    }
    lastTime = now;
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
