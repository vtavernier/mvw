#ifndef _GL_STATE_HPP_
#define _GL_STATE_HPP_

#include <shadertoy.hpp>

#include "log.hpp"
#include "uniforms.hpp"

#include "mvw/geometry.hpp"

#include "mvw_buffer.hpp"

#include "discovered_uniform.hpp"

struct gl_state {
    /// Main logger instance
    std::shared_ptr<spd::logger> log;

    /// Loaded geometry handle
    std::shared_ptr<mvw_geometry> geometry;

    /// Extra uniforms
    geometry_inputs_t extra_inputs;

    /// Global rendering context for all possible chains
    shadertoy::render_context context;
    /// Target rendering size
    shadertoy::rsize render_size;

    /**
     * Loaded chain state
     *
     * Represents a given version of a rendering swap chain.
     */
    struct chain_instance {
        shadertoy::swap_chain chain;
        shadertoy::swap_chain geometry_chain;
        std::shared_ptr<mvw_buffer> geometry_buffer;
        std::shared_ptr<shadertoy::buffers::toy_buffer> postprocess_buffer;

        parsed_inputs_t parsed_inputs;
        std::vector<discovered_uniform> discovered_uniforms;

        chain_instance(std::shared_ptr<spd::logger> log,
                       std::shared_ptr<shadertoy::compiler::program_template>
                           g_buffer_template,
                       const std::string &shader_path,
                       const std::string &postprocess_path,
                       shadertoy::render_context &context,
                       shadertoy::rsize &render_size,
                       std::shared_ptr<mvw_geometry> geometry);

        void render(shadertoy::render_context &context,
                    geometry_inputs_t &extra_inputs, bool draw_wireframe,
                    bool draw_quad, const shadertoy::rsize &render_size);

      private:
        void parse_uniforms(const std::string &path,
                            std::shared_ptr<spd::logger> log);
    };

    /// Loaded chain states
    std::vector<chain_instance> chains;

    gl_state(std::shared_ptr<spd::logger> log, int width, int height,
             const std::string &geometry_path);

    void load_chain(const std::string &shader_path,
                    const std::string &postprocess_path);

    void render(bool draw_wireframe, bool draw_quad, int back_revision = 0);

    void render_imgui(int back_revision = 0);

    void get_render_ms(float times[2], int back_revision = 0);

    const shadertoy::gl::texture &get_render_result(int back_revision = 0);

    void allocate_textures();

   private:
    std::shared_ptr<shadertoy::compiler::program_template> g_buffer_template_;
};

#endif /* _GL_STATE_HPP_ */
