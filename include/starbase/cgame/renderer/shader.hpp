#pragma once

#include <unordered_map>

#include <starbase/gl.hpp>
#include <starbase/optional.hpp>

#include <starbase/game/id.hpp>
#include <starbase/game/fwd.hpp>

namespace Starbase {

struct Shader {
	GLuint program;
	std::unordered_map<id_t, GLint> uniforms;
	std::unordered_map<id_t, GLint> attributes;

	static optional<Shader> Create( IFilesystem& fs
								  , const char* vertexShaderPath
								  , const char* fragmentShaderPath
								  , const char* const uniformNames[]
								  , const char* const attributeNames[]
								  );
};

} // namespace Starbase
