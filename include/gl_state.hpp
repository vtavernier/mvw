#ifndef _GL_STATE_HPP_
#define _GL_STATE_HPP_

#include <shadertoy.hpp>

#include "log.hpp"
#include "uniforms.hpp"

#include "mvw/geometry.hpp"

#include "mvw_buffer.hpp"

#include "discovered_uniform.hpp"

#include "options.hpp"

struct gl_state {
    /// Extra uniforms
    geometry_inputs_t extra_inputs;

    /// Global rendering context for all possible chains
    shadertoy::render_context context;
    /// Target rendering size
    shadertoy::rsize render_size;

    /// Geometry center point
    glm::vec3 center;

    /// Geometry scale
    float scale;

    /**
     * Loaded chain state
     *
     * Represents a given version of a rendering swap chain.
     */
    struct chain_instance {
        shader_program_options opt;

        shadertoy::swap_chain chain;
        shadertoy::swap_chain geometry_chain;
        std::shared_ptr<mvw_buffer> geometry_buffer;
        std::shared_ptr<shadertoy::buffers::toy_buffer> postprocess_buffer;

        parsed_inputs_t parsed_inputs;
        std::vector<discovered_uniform> discovered_uniforms;

        chain_instance(std::shared_ptr<shadertoy::compiler::program_template>
                           g_buffer_template,
                       const shader_program_options &opt,
                       shadertoy::render_context &context,
                       shadertoy::rsize &render_size);

        void render(shadertoy::render_context &context,
                    geometry_inputs_t &extra_inputs, bool draw_wireframe,
                    bool draw_quad, const shadertoy::rsize &render_size,
                    std::shared_ptr<mvw_geometry> geometry);

       private:
        void parse_uniforms(const shader_file_program &sfp);

        void compile_shader_source(const std::string &shader_path);
    };

    /// Loaded chain states
    std::vector<std::unique_ptr<chain_instance>> chains;

    gl_state(const frame_options &opt);

    void load_chain(const shader_program_options &opt);

    void load_geometry(const geometry_options &geometry);

    void render(bool draw_wireframe, bool draw_quad, int back_revision = 0);

    void render_imgui(int back_revision = 0);

    void get_render_ms(float times[2], int back_revision = 0);

    std::vector<shadertoy::members::member_output_t> get_render_result(int back_revision = 0, const std::string &target = "") const;

    const std::vector<discovered_uniform> &get_discovered_uniforms(
        int back_revision = 0) const;

    std::vector<discovered_uniform> &get_discovered_uniforms(
        int back_revision = 0);

    bool has_postprocess(int back_revision = 0) const;

    void allocate_textures();

   private:
    std::shared_ptr<shadertoy::compiler::program_template> g_buffer_template_;

    /// Loaded geometry handle
    std::shared_ptr<mvw_geometry> geometry_;
};

#endif /* _GL_STATE_HPP_ */
