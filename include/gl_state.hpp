#ifndef _GL_STATE_HPP_
#define _GL_STATE_HPP_

#include <shadertoy.hpp>

#include "log.hpp"

#include "mvw/geometry.hpp"

#include "mvw_buffer.hpp"

#include "discovered_bindings.hpp"
#include "discovered_uniform.hpp"

#include "options.hpp"

#include "data_input.hpp"

typedef std::map<std::string, std::shared_ptr<data_input>> input_map_t;

struct viewer_state;

struct gl_state {
    /// Global rendering context for all possible chains
    shadertoy::render_context context;
    /// Target rendering size
    shadertoy::rsize render_size;

    /// Bounding box details
    glm::vec3 bbox_min, bbox_max;

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

        std::vector<discovered_uniform> discovered_uniforms;
        std::vector<discovered_binding> buffer_bindings;

        bool needs_init;
        std::string error_status;

        chain_instance(std::shared_ptr<shadertoy::compiler::program_template>
                           g_buffer_template,
                       const shader_program_options &opt,
                       const input_map_t &inputs,
                       shadertoy::render_context &context,
                       shadertoy::rsize &render_size);

        void render(shadertoy::render_context &context, bool draw_wireframe,
                    const shadertoy::rsize &render_size,
                    std::shared_ptr<mvw_geometry> geometry, bool full_render);

        template <typename TKey, typename... Targs>
        void set_uniform(const TKey &identifier, Targs &&... value) const {
            if (!error_status.empty()) return;

            chain.set_uniform(identifier, std::forward<Targs...>(value...));
            geometry_chain.set_uniform(identifier, std::forward<Targs...>(value...));
        }

        void set_named(const std::string &identifier, uniform_variant value);

        void load_defaults();

        void init(shadertoy::render_context &context);

        void add_input(const std::string &name, std::shared_ptr<data_input> input);

        void allocate_textures(shadertoy::render_context &context);

       private:
        void parse_directives(const shader_file_program &sfp, bool parse_bindings);

        void compile_shader_source(const std::string &shader_path);
    };

    /// Loaded chain states
    std::vector<std::unique_ptr<chain_instance>> chains;

    gl_state(const frame_options &opt);

    void load_chain(const shader_program_options &opt);

    void load_geometry(const geometry_options &geometry);

    void render(bool draw_wireframe, int back_revision = 0,
                bool full_render = true);

    bool render_imgui(int back_revision = 0);

    void get_render_ms(float times[2], int back_revision = 0);

    std::vector<shadertoy::members::member_output_t> get_render_result(int back_revision = 0, const std::string &target = "") const;

    std::string get_render_error(int back_revision = 0) const;

    const std::vector<discovered_uniform> &get_discovered_uniforms(
        int back_revision = 0) const;

    std::vector<discovered_uniform> &get_discovered_uniforms(
        int back_revision = 0);

    bool has_postprocess(int back_revision = 0) const;

    void allocate_textures();

    void update_uniforms(float t, const viewer_state &state);

    void load_defaults();

    void set_input(const std::string &name, std::vector<float> data, std::array<uint32_t, 3> dims);

   private:
    std::shared_ptr<shadertoy::compiler::program_template> g_buffer_template_;

    /// Loaded geometry handle
    std::shared_ptr<mvw_geometry> geometry_;

    /// Named data inputs
    input_map_t inputs_;
};

#endif /* _GL_STATE_HPP_ */
