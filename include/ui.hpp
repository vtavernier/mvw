#ifndef _UI_HPP_
#define _UI_HPP_

bool libshadertoy_test_exit();

template <typename Context>
struct basic_example_ctx {
    Context context;
    shadertoy::swap_chain chain;
    shadertoy::rsize render_size;
};

typedef basic_example_ctx<shadertoy::render_context> example_ctx;

template <typename ExampleContext>
void example_set_framebuffer_size(GLFWwindow *window, int width, int height) {
    // Get the context from the window user pointer
    auto &ctx =
        *static_cast<ExampleContext *>(glfwGetWindowUserPointer(window));

    // Reallocate textures
    ctx.render_size = shadertoy::rsize(width, height);
    ctx.context.allocate_textures(ctx.chain);
}

#endif /* _UI_HPP_ */
