//
// Created by carlo on 2024-10-07.
//

#ifndef BASE_RENDERER_HPP
#define BASE_RENDERER_HPP

namespace rendering
{
class base_renderer
{
  public:
	virtual ~base_renderer()                   = default;
	virtual void recreate_swap_chain_resources() = 0;
	virtual void set_render_operation()         = 0;
	// maybe remove this
	virtual void reload_shaders() = 0;
};
}        // namespace rendering
#endif        // BASE_RENDERER_HPP
