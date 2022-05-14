#include "v_object.hpp"

namespace vengine
{
	v_object::v_object( 
							std::unique_ptr<v_mesh>& m,
							int pidx,
							std::string pname
				):mesh{m}, pool_idx_{pidx}, pipeline_name_{pname}
	{
		//revive();
	}

	v_object::~v_object()
	{
	}

	void v_object::set_visibility( bool aa, int sa, std::string pn, int tv )
	{
		always_alive = aa;
		seconds_alive = sa;
		pretty_name = pn;
		token_value = tv;
	}

	void v_object::bind( VkCommandBuffer& commandBuffer )
	{
		mesh -> bind( commandBuffer );
	}

	uint32_t v_object::count()
	{
		return mesh -> count();
	}

	bool v_object::is_alive()
	{
		const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		long age = std::chrono::duration_cast<std::chrono::seconds>( now - birth_timestamp ).count();
		return always_alive || age < seconds_alive;
	}

	void v_object::revive()
	{
		birth_timestamp = std::chrono::system_clock::now();
	}
}
