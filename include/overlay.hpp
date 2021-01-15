#ifndef OVERLAY_H
#define OVERLAY_H 1

#include <vulkan/vulkan.hpp>

struct overlay_settings {
  float bg_color[4];
  int window_width;
  int window_height;
};

// hmmm maybe introduce a struct for this?
void init_overlay(
  GLFWwindow* _win,
  vk::Device logical_device,
  vk::PhysicalDevice physical_device,
  vk::Queue graphics_queue,
  uint32_t graphics_queue_index,
  std::vector<vk::Framebuffer> framebuffers,
  vk::Format color_format,
  vk::Format depth_format
);

void resize_overlay(uint32_t framebuffer_width, uint32_t framebuffer_height);

// buffer index is the framebuffer index that is to be rendered to
// use the render finished semaphore of the main program so that
// the overlay has the chance to wait for the main programm to finish
// and then render the overlay on top. Will return a Semaphore that
// that the main program can wait vor
vk::Semaphore submit_overlay(struct overlay_settings* settings, uint32_t buffer_index, vk::Semaphore main_finished_semaphore);

// cleanup
void shutdown_overlay();

#endif
