#include "base_tools.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../misc/tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../misc/stb_image.h"

namespace vengine
{
	inline void hash_combine(std::size_t& seed) { };

	template <typename T, typename... Rest>
		inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) 
		{
			std::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
			hash_combine(seed, rest...);
		};


	std::size_t Vertex::hash() const
	{
		size_t seed = 0;
		vengine::hash_combine( seed, position, normal, color, uv );
		return seed;
	}

	VkVertexInputBindingDescription Vertex::get_binding_description()
	{
		VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription> Vertex::get_attribute_descriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);
		
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, color);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, uv);

        return attributeDescriptions;
	}

	uint32_t getMemoryTypeIndex( VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
		for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1) {
				if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}
			typeBits >>= 1;
		}
		return 0;
	}

}

namespace vengine::tools
{
	std::pair<std::vector<Vertex>,std::vector<uint32_t>> load_obj_file( std::string fname_obj )
    {
        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = ""; // Path to material files

        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(fname_obj, reader_config))
        {
            if (!reader.Error().empty())
            {
                std::cerr << "TinyObjReader: " << reader.Error();
            }
            throw std::runtime_error("Could not load " + fname_obj);
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        auto& materials = reader.GetMaterials(); 

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        int v_count = 0;

        std::map<std::size_t, uint32_t> vertex_hashes; 

        for( const auto& shape : shapes )
        {
            for( const auto &index : shape.mesh.indices )
            {
                Vertex vertex{};

                if( index.vertex_index >= 0 )
                {
                    vertex.position = {
                        attrib.vertices[ 3 * index.vertex_index + 0 ] / 88,
                        attrib.vertices[ 3 * index.vertex_index + 1 ] / 88,
                        attrib.vertices[ 3 * index.vertex_index + 2 ] / 88
                    };
 
                    vertex.normal = {
                        attrib.normals[ 3 * index.normal_index + 0 ],
                        attrib.normals[ 3 * index.normal_index + 1 ],
                        attrib.normals[ 3 * index.normal_index + 2 ]
                    };

                    vertex.uv = {
                        attrib.texcoords[ 2 * index.texcoord_index + 0 ],
                        1-attrib.texcoords[ 2 * index.texcoord_index + 1 ]
                    };

                    vertex.color = {  1.0f, 0.0f, 0.0f };
                    
                    auto v_hash = vertex.hash();

                    if( vertex_hashes.count( v_hash ) > 0 )
                    {
                        indices.push_back( vertex_hashes[v_hash] );
                    }
                    else
                    {
                        vertices.push_back( vertex );
                        indices.push_back( v_count );
                        vertex_hashes[ v_hash ] = v_count;
                        v_count++;
                    }
                }
            }
        }

        std::cout << " -- Number of loaded vertices : " << vertices.size() << std::endl;

        return { vertices, indices };
    }
	
	std::pair<std::vector<Vertex>,std::vector<uint32_t>> load_obj_memory( std::string &obj_str,
																		  std::string &mtl_str)
	{
		tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = ""; // Path to material files

        tinyobj::ObjReader reader;
        if (!reader.ParseFromString( obj_str, mtl_str, reader_config))
        {
            if (!reader.Error().empty())
            {
                std::cerr << "TinyObjReader: " << reader.Error();
            }
            throw std::runtime_error("Could not load object");
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        auto& materials = reader.GetMaterials(); 

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        int v_count = 0;

        std::map<std::size_t, uint32_t> vertex_hashes; 

        for( const auto& shape : shapes )
        {
            for( const auto &index : shape.mesh.indices )
            {
                Vertex vertex{};

                if( index.vertex_index >= 0 )
                {
                    vertex.position = {
                        attrib.vertices[ 3 * index.vertex_index + 0 ] / 88,
                        attrib.vertices[ 3 * index.vertex_index + 1 ] / 88,
                        attrib.vertices[ 3 * index.vertex_index + 2 ] / 88
                    };
 
                    vertex.normal = {
                        attrib.normals[ 3 * index.normal_index + 0 ],
                        attrib.normals[ 3 * index.normal_index + 1 ],
                        attrib.normals[ 3 * index.normal_index + 2 ]
                    };

                    vertex.uv = {
                        attrib.texcoords[ 2 * index.texcoord_index + 0 ],
                        1-attrib.texcoords[ 2 * index.texcoord_index + 1 ]
                    };

                    vertex.color = {  1.0f, 0.0f, 0.0f };
                    
                    auto v_hash = vertex.hash();

                    if( vertex_hashes.count( v_hash ) > 0 )
                    {
                        indices.push_back( vertex_hashes[v_hash] );
                    }
                    else
                    {
                        vertices.push_back( vertex );
                        indices.push_back( v_count );
                        vertex_hashes[ v_hash ] = v_count;
                        v_count++;
                    }
                }
            }
        }

        std::cout << " -- Number of loaded vertices : " << vertices.size() << std::endl;

        return { vertices, indices };
	}

	std::pair<uchar*, frame_size> load_image_file( std::string filename )
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load( filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);	

		frame_size s = {
			static_cast<uint32_t>(texWidth),
			static_cast<uint32_t>(texHeight),
			static_cast<uint32_t>(texChannels)
		};

		std::cout << s.height << "\t" << s.width << "\t" << s.channels << std::endl;

		return { pixels, s };
	}

	std::pair<uchar*, frame_size> load_image_memory( std::vector<char>& data )
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load_from_memory( reinterpret_cast<const stbi_uc*>( data.data() ), 
												 data.size(), &texWidth, &texHeight, 
												 &texChannels, STBI_rgb_alpha);	

		frame_size s = {
			static_cast<uint32_t>(texWidth),
			static_cast<uint32_t>(texHeight),
			static_cast<uint32_t>(texChannels)
		};

		std::cout << s.height << "\t" << s.width << "\t" << s.channels << std::endl;

		return { pixels, s };

	}

	std::vector<char> load_file( std::string fname )
	{
		std::fstream infile( fname, std::ios::in | std::ios::binary | std::ios::ate );
		int file_size = infile.tellg();

		std::vector<char> data( file_size );

		infile.seekp(0);
		infile.read( data.data(), file_size );

		return data;	

	}
}
