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
#include "viewer_state.hpp"

#include "options.hpp"

using namespace shadertoy;
using shadertoy::gl::gl_call;

gl_state::gl_state(const frame_options &opt)
    : g_buffer_template_(std::make_shared<compiler::program_template>()) {
    // The default vertex shader is not sufficient, we replace it with our own

    // Add LIBSHADERTOY definition
    auto preprocessor_defines(
        std::make_shared<compiler::preprocessor_defines>());
    preprocessor_defines->definitions().emplace("LIBSHADERTOY", "1");

    // Add uniform definitions
    g_buffer_template_->shader_defines().emplace("glsl", preprocessor_defines);

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
}

gl_state::chain_instance::chain_instance(
    std::shared_ptr<compiler::program_template> g_buffer_template,
    const shader_program_options &opt, shadertoy::render_context &context,
    rsize &render_size)
    : opt(opt) {
    bool has_postprocess = !opt.postprocess.empty();
    // Compile shaders
    opt.shader.invoke(
        [this](const auto &path) {
            if (this->opt.use_make) compile_shader_source(path);
        },
        [this](const auto &_source) {});

    if (has_postprocess)
        opt.postprocess.invoke(
            [this](const auto &path) {
                if (this->opt.use_make) compile_shader_source(path);
            },
            [this](const auto &source) {});

    // Parse uniforms from source
    parse_directives(opt.shader, false);
    parse_directives(opt.postprocess, true);

    // Create the geometry buffer
    geometry_buffer = std::make_shared<mvw_buffer>("geometry");
    geometry_buffer->override_program(g_buffer_template);

    opt.shader.invoke(
        [this](const auto &path) { geometry_buffer->source_file(path); },
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
            [this](const auto &path) { postprocess_buffer->source_file(path); },
            [this](const auto &source) { postprocess_buffer->source(source); });

        // Bind outputs according to the parsed definitions
        for (const auto &binding : buffer_bindings) {
            auto binding_input(
                std::make_shared<shadertoy::inputs::buffer_input>(
                    geometry_target, binding.target_name));

            binding_input->min_filter(GL_LINEAR_MIPMAP_LINEAR);
            postprocess_buffer->inputs().emplace_back(binding.uniform_name,
                                                      binding_input);
        }

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
    VLOG->debug("Initialized main swap chain");

    context.init(geometry_chain);
    VLOG->debug("Initialized geometry-only swap chain");

    // Set framerate uniforms
    set_uniform("iTimeDelta", 1.0f / 60.0f);
    set_uniform("iFrameRate", 60.0f);
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
    } else {
        if (geometry_ && geometry_->has_hint(HINT_NOLIGHT)) {
            chains.back()->set_named("dLighting", false);
        }
    }
}

void gl_state::load_geometry(const geometry_options &geometry) {
    // Load geometry
    geometry_ = make_geometry(geometry);

    if (geometry_) {
        // Compute model scale, update state
        //  Fetch dimensions of model
        geometry_->get_dimensions(bbox_min, bbox_max);
        glm::dvec3 centroid = geometry_->get_centroid();

        glm::vec3 dimensions = bbox_max - bbox_min;
        center = (bbox_max + bbox_min) / 2.f;

        if (geometry_->has_hint(HINT_NOSCALE)) {
            scale = 1.;
        } else {
            scale = 2. / dimensions.z;
        }

        VLOG->debug("Object dimensions: {}", glm::to_string(dimensions));
        VLOG->debug("Object center: {}", glm::to_string(center));
        VLOG->debug("Object centroid: {}", glm::to_string(centroid));

        if (geometry_->has_hint(HINT_NOLIGHT)) {
            for (auto &chain : chains) {
                chain->set_named("dLighting", false);
            }
        }
    }
}

void gl_state::chain_instance::render(shadertoy::render_context &context,
                                      bool draw_wireframe,
                                      const shadertoy::rsize &render_size,
                                      std::shared_ptr<mvw_geometry> geometry,
                                      bool full_render) {
    // Set the geometry reference
    geometry_buffer->geometry(geometry);

    if (!geometry) {
        VLOG->warn("No geometry to render");
        return;
    }

    // Find if we are rendering as a quad
    bool draw_quad = false;
    for (const auto &du : discovered_uniforms) {
        if (du.s_name.compare("dQuad") == 0) {
            if (auto p = std::get_if<bool>(&du.value); p) {
                draw_quad = *p;
            }

            break;
        }
    }

    geometry_buffer->render_quad(draw_quad);

    // First call: draw the shaded geometry
    // Render the swap chain
    if (full_render) {
        chain.render(context);
    } else {
        // Render result is already ok, just render the current texture to the
        // screen
        chain.render(context, chain.members().back(), chain.members().back());
    }

    if (draw_wireframe) {
        // Copy the gl_buffer depth data onto the back left fb
        gl_call(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, 0);
        gl_call(glBindFramebuffer, GL_READ_FRAMEBUFFER,
                GLuint(geometry_buffer->target_fbo()));
        gl_call(glBlitFramebuffer, 0, 0, render_size.width, render_size.height,
                window_width, 0, render_size.width + window_width,
                render_size.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        // Second call: render wireframe on top without post-processing
        geometry_chain.set_uniform("bWireframe", 1);

        context.render(geometry_chain);

        // Note that both chains share the same program, so restore the wireframe value
        geometry_chain.set_uniform("bWireframe", 0);
    }
}

void gl_state::chain_instance::set_named(const std::string &identifier, ::uniform_variant value) {
    for (auto &du : discovered_uniforms) {
        if (du.s_name.compare(identifier) == 0) {
            du.value = value;
            du.set_uniform(*this);
            break;
        }
    }
}

void gl_state::chain_instance::load_defaults() {
    for (auto &du : discovered_uniforms) {
        du.value = du.s_def;
    }
}

void gl_state::chain_instance::parse_directives(const shader_file_program &sfp, bool parse_bindings) {
    if (sfp.empty()) return;

    std::string line;
    auto ifs(sfp.open());
    while (!ifs->eof()) {
        std::getline(*ifs, line);
        if (!try_parse_uniform(line, discovered_uniforms) && parse_bindings)
            try_parse_binding(line, buffer_bindings);
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

void gl_state::render(bool draw_wireframe, int back_revision,
                      bool full_render) {
    chains.at(chains.size() + back_revision - 1)
        ->render(context, draw_wireframe, render_size,
                 geometry_, full_render);
}

bool gl_state::render_imgui(int back_revision) {
    auto &chain = chains.at(chains.size() + back_revision - 1);
    bool changed = false;

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
            changed |= uniform->render_imgui();
            uniform->set_uniform(*chain);
        }

        ImGui::Separator();
    }

    return changed;
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

void gl_state::update_uniforms(float t, const viewer_state &state) {
    // Update model, view and projection matrices
    glm::mat4 mModel = state.get_model();
    glm::mat4 mView = state.get_view();
    // Projection matrix display range : 0.1 unit <-> 100 units
    glm::mat4 mProj = glm::perspective(
        glm::radians(25.0f),
        (float)render_size.width / (float)render_size.height, 0.1f, 100.0f);

    for (auto &chain : chains) {
        chain->set_uniform("iTime", t);
        chain->set_uniform("iFrame", state.frame_count);

        chain->set_uniform("mModel", mModel);
        chain->set_uniform("mView", mView);
        chain->set_uniform("mProj", mProj);

        chain->set_uniform("bboxMax", bbox_max);
        chain->set_uniform("bboxMin", bbox_min);
    }
}

void gl_state::load_defaults() {
    bool hint_nolight = geometry_ && geometry_->has_hint(HINT_NOLIGHT);

    for (auto &chain : chains) {
        chain->load_defaults();

        if (hint_nolight)
            chain->set_named("dLighting", false);
    }
}
