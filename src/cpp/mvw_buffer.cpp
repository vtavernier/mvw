#include "mvw_buffer.hpp"

using namespace shadertoy;

void mvw_buffer::render_geometry(const render_context &context,
                                 const io_resource &io)
{
    if (render_quad_) {
        context.screen_quad().render(time_delta_query());
    } else {
        geometry_buffer::render_geometry(context, io);
    }
}

mvw_buffer::mvw_buffer(const std::string &id)
    : geometry_buffer(id),
    render_quad_(false)
{
}
