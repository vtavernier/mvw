#include <epoxy/gl.h>

#include <shadertoy.hpp>

#include "config.hpp"
#include "gl_state.hpp"

using namespace shadertoy;
using shadertoy::gl::gl_call;

gl_state::gl_state(std::shared_ptr<spd::logger> log, int width, int height,
                   const std::string &geometry_path)
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
    geometry_buffer = std::make_shared<mvw_buffer>("geometry");
    geometry_buffer->override_program(g_buffer_template);

    geometry_buffer->source_file(shader_path);

    bool has_postprocess = !postprocess_path.empty();

    // Add the geometry buffer to the swap chain, at the given size
    auto geometry_target =
        chain.emplace_back(geometry_buffer, make_size_ref(render_size),
                           member_swap_policy::single_buffer);

    // Set the clearing details for the geometry target
    auto &gs(geometry_target->state());

    // Without a background, the buffer should also clear the previous contents
    gs.clear_color({0.0f, 0.0f, 0.0f, 0.0f});
    gs.clear_depth(1.f);
    gs.clear_bits(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth test since we are rendering geometry
    gs.enable(GL_DEPTH_TEST);

    // Add the geometry buffer to the geometry-only chain
    // TODO: set viewport for geometry_chain
    auto wireframe_target =
        geometry_chain.emplace_back(geometry_buffer, make_size_ref(render_size),
                                    member_swap_policy::default_framebuffer);

    // Set parameters for the wireframe target
    auto &ws(wireframe_target->state());

    // LEQUAL depth testing to draw over already rendered geometry
    ws.enable(GL_DEPTH_TEST);
    ws.depth_func(GL_LEQUAL);

    // Smooth wireframes
    ws.enable(GL_LINE_SMOOTH);
    ws.polygon_mode(GL_LINE);

    if (has_postprocess) {
        // Add the postprocess buffer
        postprocess_buffer =
            std::make_shared<buffers::toy_buffer>("postprocess");
        postprocess_buffer->source_file(postprocess_path);

        // The postprocess pass has the output of the geometry pass as input 0
        auto postprocess_input(
            std::make_shared<shadertoy::inputs::buffer_input>(geometry_target));
        postprocess_input->min_filter(GL_LINEAR_MIPMAP_LINEAR);
        postprocess_buffer->inputs().emplace_back(postprocess_input);

        // Render the output of the postprocess pass to the default framebuffer
        auto postprocess_member =
            chain.emplace_back(postprocess_buffer, make_size_ref(render_size),
                               member_swap_policy::single_buffer);

        // Clear the background of the postprocess buffer
        postprocess_member->state().clear_color({0.15f, 0.15f, 0.15f, 1.f});
        postprocess_member->state().clear_bits(GL_COLOR_BUFFER_BIT);

        postprocess_member->state().enable(GL_BLEND);

        postprocess_member->state().blend_mode_rgb(GL_FUNC_ADD);
        postprocess_member->state().blend_src_rgb(GL_SRC_ALPHA);
        postprocess_member->state().blend_dst_rgb(GL_ONE_MINUS_SRC_ALPHA);

        postprocess_member->state().blend_mode_alpha(GL_FUNC_ADD);
        postprocess_member->state().blend_src_alpha(GL_SRC_ALPHA);
        postprocess_member->state().blend_dst_alpha(GL_ONE_MINUS_SRC_ALPHA);
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
                                      bool draw_wireframe,
                                      bool draw_quad,
                                      const shadertoy::rsize &render_size) {
    // Update quad rendering status
    geometry_buffer->render_quad(draw_quad);
    extra_inputs.get<dQuad>() = draw_quad ? 1 : 0;

    // First call: draw the shaded geometry
    // Render the swap chain
    context.render(chain);

    if (draw_wireframe) {
        // Copy the gl_buffer depth data onto the back left fb
        gl_call(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, 0);
        gl_call(glBindFramebuffer, GL_READ_FRAMEBUFFER, GLuint(geometry_buffer->target_fbo()));
        gl_call(glBlitFramebuffer, 0, 0, render_size.width, render_size.height,
                                   window_width, 0, render_size.width + window_width, render_size.height,
                                   GL_DEPTH_BUFFER_BIT,
                                   GL_NEAREST);

        // Second call: render wireframe on top without post-processing
        extra_inputs.get<bWireframe>() = GL_TRUE;
        // Render swap chain
        context.render(geometry_chain);
        // Restore bWireframe state
        extra_inputs.get<bWireframe>() = GL_FALSE;
    }
}

void gl_state::render(bool draw_wireframe, bool draw_quad, int back_revision) {
    chains.at(chains.size() + back_revision - 1)
        .render(context, extra_inputs, draw_wireframe, draw_quad, render_size);
}

void gl_state::get_render_ms(float times[2], int back_revision) {
    auto &chain(chains.at(chains.size() + back_revision - 1));

    times[0] = chain.geometry_buffer->elapsed_time() / 1.0e6f;

    if (chain.postprocess_buffer)
        times[1] = chain.postprocess_buffer->elapsed_time() / 1.0e6f;
    else
        times[1] = 0.0f;
}

const gl::texture &gl_state::get_render_result(int back_revision) {
    auto &chain(chains.at(chains.size() + back_revision - 1));
    auto member(std::static_pointer_cast<members::buffer_member>(*++chain.chain.members().rbegin()));

    log->info("Fetching frame({}) rev {}", member->buffer()->id(), back_revision);

    return *member->output();
}

void gl_state::allocate_textures() {
    for (auto &chain : chains) {
        context.allocate_textures(chain.chain);
        context.allocate_textures(chain.geometry_chain);
    }
}
