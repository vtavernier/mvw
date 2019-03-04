#include "shadertoy/uniform_state_decl.hpp"

DECLARE_UNIFORM(glm::mat4, mModel, "mat4");
DECLARE_UNIFORM(glm::mat4, mView, "mat4");
DECLARE_UNIFORM(glm::mat4, mProj, "mat4");
DECLARE_UNIFORM(GLint, bWireframe, "bool");

DECLARE_UNIFORM(glm::vec3, bboxMax, "vec3");
DECLARE_UNIFORM(glm::vec3, bboxMin, "vec3");

DECLARE_UNIFORM(GLint, dQuad, "bool");

DECLARE_DYNAMIC_UNIFORM(iMvwParsedUniforms, float, glm::vec2, glm::vec3,
                        glm::vec4, int, glm::ivec2, glm::ivec3, glm::ivec4,
                        bool, glm::bvec2, glm::bvec3, glm::bvec4);

// We need a custom inputs type to pass the MVP matrix
typedef shadertoy::shader_inputs<
        mModel,
        mView,
        mProj,
        bWireframe,
        bboxMax,
        bboxMin,
        dQuad
    > geometry_inputs_t;

// The parsed uniforms are separate because they depend on the shader revision
typedef shadertoy::shader_inputs<iMvwParsedUniforms> parsed_inputs_t;
