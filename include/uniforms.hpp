#include "shadertoy/uniform_state_decl.hpp"

DECLARE_UNIFORM(glm::mat4, mModel, "mat4");
DECLARE_UNIFORM(glm::mat4, mView, "mat4");
DECLARE_UNIFORM(glm::mat4, mProj, "mat4");
DECLARE_UNIFORM(GLint, bWireframe, "bool");

DECLARE_UNIFORM(glm::vec3, bboxMax, "vec3");
DECLARE_UNIFORM(glm::vec3, bboxMin, "vec3");

DECLARE_UNIFORM(GLfloat, gTilesize, "float");
DECLARE_UNIFORM(GLint, gSplats, "int");
DECLARE_UNIFORM(GLfloat, gF0, "float");
DECLARE_UNIFORM(glm::vec2, gW0, "vec2");

DECLARE_UNIFORM(GLfloat, cFilterLod, "float");

DECLARE_UNIFORM(GLint, dGrid, "bool");

// We need a custom inputs type to pass the MVP matrix
typedef shadertoy::shader_inputs<
        mModel,
        mView,
        mProj,
        bWireframe,
        bboxMax,
        bboxMin,
        gTilesize,
        gSplats,
        gF0,
        gW0,
        cFilterLod,
        dGrid
    > geometry_inputs_t;
