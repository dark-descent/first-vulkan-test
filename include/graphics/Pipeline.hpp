#ifndef ENGINE_GRAPHICS_PIPELINE_HPP
#define ENGINE_GRAPHICS_PIPELINE_HPP

#include "graphics/GfxObject.hpp"
#include "framework.hpp"

namespace NovaEngine::Graphics
{
	enum class ShaderType
	{
		VERTEX,
		FRAGMENT,
	};

	class Pipeline : public GfxObject<const char*, const char*>
	{
	private:
		VkShaderModule loadShader(const char* shaderFileName, ShaderType type);

		VkPipelineLayout layout_;
		VkPipeline pipeline_;
		GFX_CTOR(Pipeline), layout_(VK_NULL_HANDLE) {}

	protected:
		bool onInitialize(const char* vertexShader, const char* fragmentShader);
		bool onTerminate();

	public:
		VkPipeline& operator*() { return pipeline_; }
	};

};

#endif
