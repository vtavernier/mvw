#include "shadertoy/uniform_state_decl.hpp"

DECLARE_UNIFORM(glm::mat4, mModel, "mat4");
DECLARE_UNIFORM(glm::mat4, mView, "mat4");
DECLARE_UNIFORM(glm::mat4, mProj, "mat4");
DECLARE_UNIFORM(GLint, bWireframe, "bool");

// We need a custom inputs type to pass the MVP matrix
typedef shadertoy::shader_inputs<mModel, mView, mProj, bWireframe> geometry_inputs_t;
