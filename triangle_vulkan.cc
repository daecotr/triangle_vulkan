#include <cstdlib>
#include <iostream>
#include <unordered_set>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace tvk {

class Exception : public std::exception {
protected:
  std::string message;

public:
  explicit Exception(const std::string &message) : message{message} {}
  const char *what() const noexcept override { return message.c_str(); }
};

} // namespace tvk

int main(int argc, char *argv[]) {
  try {
    // Init GLFW, handle errors
    if (!glfwInit())
      throw tvk::Exception{"Failed to initialize GLFW"};

    glfwSetErrorCallback([](int error, const char *description) {
      throw tvk::Exception{description};
    });

    // Set GLFW window params
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    // Create window
    GLFWwindow *window = glfwCreateWindow(512, 512, "", nullptr, nullptr);
    if (!window)
      throw tvk::Exception("Failed to create GLFW window");

    // Check available Vulkan layers
    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    std::vector<vk::LayerProperties> availableLayers =
        vk::enumerateInstanceLayerProperties();
    std::unordered_set<std::string> availableLayerNames;
    for (const auto &layer : availableLayers)
      availableLayerNames.insert(layer.layerName);

    std::cout << "Available layers" << std::endl;
    for (const auto &layer : availableLayerNames)
      std::cout << "- " << layer << std::endl;

    std::vector<const char *> enabledLayers;
    for (const auto &layer : validationLayers) {
      if (availableLayerNames.contains(layer))
        enabledLayers.push_back(layer);
      else
        std::cerr << "Validation layer not available: " << layer << std::endl;
    }

    // Query instance extensions
    std::cout << "Available extensions" << std::endl;
    auto availableExtensions = vk::enumerateInstanceExtensionProperties();
    for (const auto &extension : availableExtensions)
      std::cout << "- " << extension.extensionName << std::endl;

    // Add required extensions
    std::vector<const char *> extensions = {};
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    extensions.insert(extensions.end(), glfwExtensions,
                      glfwExtensions + glfwExtensionCount);

    // Create Vulkan instance
    vk::ApplicationInfo appInfo("", 1, "No Engine", 1, VK_API_VERSION_1_3);
    vk::InstanceCreateInfo instanceInfo({}, &appInfo, enabledLayers.size(),
                                        enabledLayers.data(), extensions.size(),
                                        extensions.data());
    vk::Instance instance;
    if (vk::createInstance(&instanceInfo, nullptr, &instance) !=
        vk::Result::eSuccess)
      throw tvk::Exception("Failed to create Vulkan instance");

    // Create window surface
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &rawSurface) !=
        VK_SUCCESS)
      throw tvk::Exception("Failed to create window surface");
    vk::SurfaceKHR surface = rawSurface;

    // Window cycle
    for (size_t i = 0; (i < 128) && !glfwWindowShouldClose(window); ++i) {
      glfwPollEvents();
    }

    // Cleanup
    instance.destroySurfaceKHR(surface);
    instance.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
  } catch (tvk::Exception err) {
    std::cout << "Exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
  } catch (vk::SystemError &err) {
    std::cerr << "vk::SystemError: " << err.what() << std::endl;
    return EXIT_FAILURE;
  } catch (std::exception &err) {
    std::cerr << "std::exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "unknown error\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
