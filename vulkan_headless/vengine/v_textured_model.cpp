#include "v_textured_model.hpp"

namespace vengine
{
	v_textured_model::v_textured_model( 
							std::unique_ptr<v_mesh>& m,
							int pidx,
							std::string pname
				):mesh{m}, pool_idx_{pidx}, pipeline_name_{pname}
	{
	}

	v_textured_model::~v_textured_model()
	{
	}

	void v_textured_model::bind( VkCommandBuffer& commandBuffer )
	{
		mesh -> bind( commandBuffer );
	}

	uint32_t v_textured_model::count()
	{
		return mesh -> count();
	}
}
