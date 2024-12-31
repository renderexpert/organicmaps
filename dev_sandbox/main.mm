#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if __APPLE__
  #define GLFW_EXPOSE_NATIVE_COCOA
#else
  #error Unsupported OS
#endif

#import <Cocoa/Cocoa.h>
#include <GLFW/glfw3native.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

void * createCocoaWindowView(GLFWwindow * window) 
{
  NSWindow * nswindow = glfwGetCocoaWindow(window);
  CAMetalLayer * layer = [CAMetalLayer layer];
  layer.device = MTLCreateSystemDefaultDevice();
  layer.opaque = YES;
  layer.displaySyncEnabled = YES;
  NSScreen * screen = [NSScreen mainScreen];
  CGFloat factor = [screen backingScaleFactor];
  layer.contentsScale = factor;
  nswindow.contentView.layer = layer;
  nswindow.contentView.wantsLayer = YES;
  return nswindow.contentView;
}

void updateContentScale(GLFWwindow * window, float scale) 
{
  NSWindow * nswindow = glfwGetCocoaWindow(window);
  nswindow.contentView.layer.contentsScale = scale;
}
