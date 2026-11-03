#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <vector>

namespace game
{
struct Vertex
{
  glm::vec3 position;
  glm::vec3 color;
  glm::vec2 uv;
};

struct Mesh
{
  Mesh(std::vector<Vertex> _vertices, std::vector<uint16_t> _indices)
    : vertices(std::move(_vertices)), indices(std::move(_indices))
  {
    VkBufferCreateInfo vertexCreateInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = vertices.size() * sizeof(Vertex),
      .usage = VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
  }

  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;

  VkBuffer vertexBuffer;
  VmaAllocation vertexAllocation;

  VkBuffer indexBuffer;
  VmaAllocation indexAllocation;
};
}
