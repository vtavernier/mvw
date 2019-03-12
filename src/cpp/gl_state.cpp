#include <epoxy/gl.h>

#include <shadertoy.hpp>

#include <fstream>
#include <regex>

#include <glm/gtx/string_cast.hpp>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "imgui.h"

#include "config.hpp"
#include "gl_state.hpp"

#include "options.hpp"

using namespace shadertoy;
using shadertoy::gl::gl_call;

gl_state::gl_state(const frame_options &opt)
    : g_buffer_template_(std::make_shared<compiler::program_template>()) {
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
        compiler::shader_template::parse_file(SHADERS_BASE "/vertex.glsl"));

    // Same for fragment shader
    g_buffer_template_->emplace(
        GL_FRAGMENT_SHADER,
        compiler::shader_template::parse_file(SHADERS_BASE "/fragment.glsl"));

    // Force compilation of new template
    g_buffer_template_->compile(GL_VERTEX_SHADER);

    // Set the context parameters (render size and some uniforms)
    render_size = rsize(opt.width, opt.height);
    context.state().get<iTimeDelta>() = 1.0 / 60.0;
    context.state().get<iFrameRate>() = 60.0;
}

gl_state::chain_instance::chain_instance(
    std::shared_ptr<compiler::program_template> g_buffer_template,
    const shader_program_options &opt, shadertoy::render_context &context,
    rsize &render_size)
    : opt(opt) {
    bool has_postprocess = !opt.postprocess.empty();

    // There is some shared state in the template here, but it's the simplest
    g_buffer_template->shader_inputs()["parsed"] = &parsed_inputs;
    context.buffer_template().shader_inputs()["parsed"] = &parsed_inputs;

    // Parse uniforms from source
    parse_uniforms(opt.shader);
    parse_uniforms(opt.postprocess);

    // Apply the defaults to define the uniforms before the template is
    // generated
    for (auto &uniform : discovered_uniforms) {
        uniform.create_uniform(parsed_inputs);
    }

    // Create the geometry buffer
    geometry_buffer = std::make_shared<mvw_buffer>("geometry");
    geometry_buffer->override_program(g_buffer_template);

    opt.shader.invoke(
        [this](const auto &path) {
            if (this->opt.use_make) compile_shader_source(path);
            geometry_buffer->source_file(path);
        },
        [this](const auto &source) { geometry_buffer->source(source); });

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

        opt.postprocess.invoke(
            [this](const auto &path) {
                if (this->opt.use_make) compile_shader_source(path);
                postprocess_buffer->source_file(path);
            },
            [this](const auto &source) { postprocess_buffer->source(source); });

        // The postprocess pass has the output of the geometry pass as input 0
        auto postprocess_input(
            std::make_shared<shadertoy::inputs::buffer_input>(geometry_target, 0));
        postprocess_input->min_filter(GL_LINEAR_MIPMAP_LINEAR);
        postprocess_buffer->inputs().emplace_back(postprocess_input);

        auto lighting_input(
            std::make_shared<shadertoy::inputs::buffer_input>(geometry_target, 1));
        postprocess_buffer->inputs().emplace_back(lighting_input);

        // Add postprocess pass to the chain
        chain.emplace_back(postprocess_buffer, make_size_ref(render_size),
                           member_swap_policy::single_buffer);
    }

    auto screen_member =
        members::make_screen(window_width, 0, make_size_ref(render_size));
    chain.push_back(screen_member);

    // Clear the background before rendering on screen
    screen_member->state().clear_color({0.15f, 0.15f, 0.15f, 1.f});
    screen_member->state().clear_bits(GL_COLOR_BUFFER_BIT);

    screen_member->state().enable(GL_BLEND);

    screen_member->state().blend_mode_rgb(GL_FUNC_ADD);
    screen_member->state().blend_src_rgb(GL_SRC_ALPHA);
    screen_member->state().blend_dst_rgb(GL_ONE_MINUS_SRC_ALPHA);

    screen_member->state().blend_mode_alpha(GL_FUNC_ADD);
    screen_member->state().blend_src_alpha(GL_SRC_ALPHA);
    screen_member->state().blend_dst_alpha(GL_ONE_MINUS_SRC_ALPHA);

    // Initialize context
    context.init(chain);
    VLOG->info("Initialized main swap chain");

    context.init(geometry_chain);
    VLOG->info("Initialized geometry-only swap chain");
}

void gl_state::load_chain(const shader_program_options &opt) {
    bool migrate_uniforms = !chains.empty();

    chains.emplace_back(std::make_unique<chain_instance>(
        g_buffer_template_, opt, context, render_size));

    if (migrate_uniforms) {
        auto &chain_now = chains.back();
        auto &chain_before = *++chains.rbegin();

        for (auto &uniform : chain_before->discovered_uniforms) {
            for (auto &new_uniform : chain_now->discovered_uniforms) {
                if (uniform.s_name == new_uniform.s_name) {
                    try_set_variant(new_uniform.value, uniform.value);
                }
            }
        }
    }
}

void gl_state::load_geometry(const geometry_options &geometry) {
    // Load geometry
    geometry_ = make_geometry(geometry);

    if (geometry_) {
        // Compute model scale, update state
        //  Fetch dimensions of model
        glm::vec3 bbox_min, bbox_max;
        geometry_->get_dimensions(bbox_min, bbox_max);
        glm::dvec3 centroid = geometry_->get_centroid();

        glm::vec3 dimensions = bbox_max - bbox_min;
        center = (bbox_max + bbox_min) / 2.f;
        scale = 2. / dimensions.z;
        VLOG->info("Object dimensions: {}", glm::to_string(dimensions));
        VLOG->info("Object center: {}", glm::to_string(center));
        VLOG->info("Object centroid: {}", glm::to_string(centroid));

        //  Set state_
        extra_inputs.get<bboxMax>() = bbox_max;
        extra_inputs.get<bboxMin>() = bbox_min;
    }
}

void gl_state::chain_instance::render(shadertoy::render_context &context,
                                      geometry_inputs_t &extra_inputs,
                                      bool draw_wireframe, bool draw_quad,
                                      const shadertoy::rsize &render_size,
                                      std::shared_ptr<mvw_geometry> geometry) {
    // Set the geometry reference
    geometry_buffer->geometry(geometry);

    if (!geometry) {
        VLOG->warn("No geometry to render");
        return;
    }

    // Update quad rendering status
    geometry_buffer->render_quad(draw_quad);
    extra_inputs.get<dQuad>() = draw_quad ? 1 : 0;

    // First call: draw the shaded geometry
    // Render the swap chain
    context.render(chain);

    if (draw_wireframe) {
        // Copy the gl_buffer depth data onto the back left fb
        gl_call(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, 0);
        gl_call(glBindFramebuffer, GL_READ_FRAMEBUFFER,
                GLuint(geometry_buffer->target_fbo()));
        gl_call(glBlitFramebuffer, 0, 0, render_size.width, render_size.height,
                window_width, 0, render_size.width + window_width,
                render_size.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        // Second call: render wireframe on top without post-processing
        extra_inputs.get<bWireframe>() = GL_TRUE;
        // Render swap chain
        context.render(geometry_chain);
        // Restore bWireframe state
        extra_inputs.get<bWireframe>() = GL_FALSE;
    }
}

void gl_state::chain_instance::parse_uniforms(const shader_file_program &sfp) {
    if (sfp.empty()) return;

    std::string line;
    auto ifs(sfp.open());
    while (!ifs->eof()) {
        std::getline(*ifs, line);
        try_parse_uniform(line, discovered_uniforms);
    }
}

void gl_state::chain_instance::compile_shader_source(
    const std::string &shader_path) {
    if (shader_path.empty()) return;

    VLOG->info("Compiling {} using make", shader_path);

    int pid = fork();

    if (pid == 0) {
        size_t begin;
        if ((begin = shader_path.find_last_of('/')) != std::string::npos) {
            begin++;
        } else {
            begin = 0;
        }

        std::string basename(shader_path.begin() + begin, shader_path.end());
        const char *args[] = {"make", basename.c_str(), NULL};

        execvp("make", const_cast<char *const *>(args));
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
            throw std::runtime_error("Failed to compile shader source code");
        }
    }
}

void gl_state::render(bool draw_wireframe, bool draw_quad, int back_revision) {
    chains.at(chains.size() + back_revision - 1)
        ->render(context, extra_inputs, draw_wireframe, draw_quad, render_size,
                 geometry_);
}

void gl_state::render_imgui(int back_revision) {
    auto &chain = chains.at(chains.size() + back_revision - 1);

    // Group by category
    std::map<std::string, std::vector<discovered_uniform *>> uniform_categories;
    for (auto &uniform : chain->discovered_uniforms) {
        auto it = uniform_categories.find(uniform.s_cat);
        if (it == uniform_categories.end()) {
            uniform_categories.emplace(
                uniform.s_cat, std::vector<discovered_uniform *>{&uniform});
        } else {
            it->second.push_back(&uniform);
        }
    }

    for (auto it = uniform_categories.begin(); it != uniform_categories.end();
         ++it) {
        ImGui::Text("%s", it->first.c_str());

        for (auto &uniform : it->second) {
            uniform->render_imgui();
            uniform->set_uniform(chain->parsed_inputs);
        }

        ImGui::Separator();
    }
}

void gl_state::get_render_ms(float times[2], int back_revision) {
    auto &chain(chains.at(chains.size() + back_revision - 1));

    times[0] = chain->geometry_buffer->elapsed_time() / 1.0e6f;

    if (chain->postprocess_buffer)
        times[1] = chain->postprocess_buffer->elapsed_time() / 1.0e6f;
    else
        times[1] = 0.0f;
}

std::vector<shadertoy::members::member_output_t> gl_state::get_render_result(int back_revision, const std::string &target) const {
    auto &chain(chains.at(chains.size() + back_revision - 1));
    std::shared_ptr<members::buffer_member> member;

    if (target.empty()) {
        member = std::static_pointer_cast<members::buffer_member>(
            *++chain->chain.members().rbegin());
    } else {
        auto it = std::find_if(
            chain->chain.members().begin(), chain->chain.members().end(),
            [&target](const auto &member) {
                if (auto buffer_member =
                        std::dynamic_pointer_cast<members::buffer_member>(
                            member)) {
                    return buffer_member->buffer()->id() == target;
                }

                return false;
            });

        if (it == chain->chain.members().end())
            throw std::runtime_error(target + " member not found");

        member = std::static_pointer_cast<members::buffer_member>(*it);
    }

    VLOG->info("Fetching frame({}) rev {}", member->buffer()->id(),
               back_revision);

    return member->output();
}

const std::vector<discovered_uniform> &gl_state::get_discovered_uniforms(
    int back_revision) const {
    return chains.at(chains.size() + back_revision - 1)->discovered_uniforms;
}

std::vector<discovered_uniform> &gl_state::get_discovered_uniforms(
    int back_revision) {
    return chains.at(chains.size() + back_revision - 1)->discovered_uniforms;
}

bool gl_state::has_postprocess(int back_revision) const {
    return !chains.at(chains.size() + back_revision - 1)
                ->opt.postprocess.empty();
}

void gl_state::allocate_textures() {
    for (auto &chain : chains) {
        context.allocate_textures(chain->chain);
        context.allocate_textures(chain->geometry_chain);
    }
}
