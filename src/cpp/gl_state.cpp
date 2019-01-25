#include <epoxy/gl.h>

#include <shadertoy.hpp>

#include "config.hpp"
#include "gl_state.hpp"

using namespace shadertoy;
using shadertoy::gl::gl_call;

gl_state::gl_state(std::shared_ptr<spd::logger> log, int width, int height, const std::string &geometry_path)
    : log(log),
      g_buffer_template_(std::make_shared<compiler::program_template>()) {
    // Register the custom inputs with the buffer template
    context.buffer_template().shader_inputs().emplace("geometry",
                                                      &extra_inputs);

    // Recompile the buffer template
    context.buffer_template().compile(GL_VERTEX_SHADER);

    // The default vertex shader is not sufficient, we replace it with our own

    // Add LIBSHADERTOY definition
    auto preprocessor_defines(
        std::make_shared<compiler::preprocessor_defines>());
    preprocessor_defines->definitions().emplace("LIBSHADERTOY", "1");

    // Add uniform definitions
    g_buffer_template_->shader_defines().emplace("glsl", preprocessor_defines);
    g_buffer_template_->shader_inputs().emplace("shadertoy", &context.state());
    g_buffer_template_->shader_inputs().emplace("geometry", &extra_inputs);

    // Load customized shader templates
    g_buffer_template_->emplace(
        GL_VERTEX_SHADER,
        compiler::shader_template::parse_file("../shaders/vertex.glsl"));

    // Same for fragment shader
    g_buffer_template_->emplace(
        GL_FRAGMENT_SHADER,
        compiler::shader_template::parse_file("../shaders/fragment.glsl"));

    // Force compilation of new template
    g_buffer_template_->compile(GL_VERTEX_SHADER);

    // Set the context parameters (render size and some uniforms)
    render_size = rsize(width - window_width, height);
    context.state().get<iTimeDelta>() = 1.0 / 60.0;
    context.state().get<iFrameRate>() = 60.0;

    // Load geometry
    log->info("Loading model {}", geometry_path);
    geometry = make_geometry(geometry_path);
}

gl_state::chain_instance::chain_instance(
    std::shared_ptr<spd::logger> log,
    std::shared_ptr<compiler::program_template> g_buffer_template,
    const std::string &shader_path, const std::string &postprocess_path,
    shadertoy::render_context &context, rsize &render_size,
    std::shared_ptr<mvw_geometry> geometry) {
    // Create the geometry buffer
    geometry_buffer = std::make_shared<buffers::geometry_buffer>("geometry");
    geometry_buffer->override_program(g_buffer_template);

    geometry_buffer->source_file(shader_path);

    // Without a background, the buffer should also clear the previous contents
    geometry_buffer->clear_color({.15f, .15f, .15f, 1.f});
    geometry_buffer->clear_depth(1.f);
    geometry_buffer->clear_bits(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool has_postprocess = !postprocess_path.empty();

    // Add the geometry buffer to the swap chain, at the given size
    auto geometry_target =
        chain.emplace_back(geometry_buffer, make_size_ref(render_size),
                           member_swap_policy::single_buffer);

    // Add the geometry buffer to the geometry-only chain
    // TODO: set viewport for geometry_chain
    geometry_chain.emplace_back(geometry_buffer, make_size_ref(render_size),
                                member_swap_policy::default_framebuffer);

    if (has_postprocess) {
        // Add the postprocess buffer
        auto postprocess_buffer =
            std::make_shared<buffers::toy_buffer>("postprocess");
        postprocess_buffer->source_file(postprocess_path);

        // The postprocess pass has the output of the geometry pass as input 0
        auto postprocess_input(
            std::make_shared<shadertoy::inputs::buffer_input>(geometry_target));
        postprocess_input->min_filter(GL_LINEAR_MIPMAP_LINEAR);
        postprocess_buffer->inputs().emplace_back(postprocess_input);

        // Render the output of the postprocess pass to the default framebuffer
        chain.emplace_back(postprocess_buffer, make_size_ref(render_size),
                           member_swap_policy::single_buffer);
    }

    chain.push_back(
        members::make_screen(window_width, 0, make_size_ref(render_size)));

    // Initialize context
    context.init(chain);
    log->info("Initialized main swap chain");

    context.init(geometry_chain);
    log->info("Initialized geometry-only swap chain");

    // Set the geometry reference
    geometry_buffer->geometry(geometry);
}

void gl_state::load_chain(const std::string &shader_path,
                          const std::string &postprocess_path) {
    chains.emplace_back(log, g_buffer_template_, shader_path, postprocess_path,
                        context, render_size, geometry);
}

void gl_state::chain_instance::render(shadertoy::render_context &context,
                                      geometry_inputs_t &extra_inputs,
                                      bool draw_wireframe) {
    // First call: clear everything
    geometry_buffer->clear_bits(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the swap chain
    gl_call(glPolygonMode, GL_FRONT_AND_BACK, GL_FILL);
    context.render(chain);

    if (draw_wireframe) {
        // Second call: render wireframe on top without post-processing
        extra_inputs.get<bWireframe>() = GL_TRUE;
        geometry_buffer->clear_bits(0);
        // Render swap chain
        gl_call(glPolygonMode, GL_FRONT_AND_BACK, GL_LINE);
        context.render(geometry_chain);
    }
}

void gl_state::render(bool draw_wireframe, int back_revision)
{
    chains.at(chains.size() + back_revision - 1).render(context, extra_inputs, draw_wireframe);
}

void gl_state::allocate_textures()
{
    for (auto &chain : chains) {
        context.allocate_textures(chain.chain);
        context.allocate_textures(chain.geometry_chain);
    }
}
