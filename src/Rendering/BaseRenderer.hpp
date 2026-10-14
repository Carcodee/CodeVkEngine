//
// Created by carlo on 2024-10-07.
//

#ifndef BASE_RENDERER_HPP
#define BASE_RENDERER_HPP

namespace rendering
{
class Base_Renderer
{
  public:
	virtual ~Base_Renderer()                   = default;
	virtual void Recreate_Swap_Chain_Resources() = 0;
	virtual void Set_Render_Operation()         = 0;
	// maybe remove this
	virtual void Reload_Shaders() = 0;
};
}        // namespace rendering
#endif        // BASE_RENDERER_HPP
