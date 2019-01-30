#ifndef _MVW_BUFFER_HPP_
#define _MVW_BUFFER_HPP_

#include <shadertoy.hpp>

class mvw_buffer : public shadertoy::buffers::geometry_buffer {
    bool render_quad_;

   protected:
    void render_geometry(const shadertoy::render_context &context,
                         const shadertoy::io_resource &io);

   public:
    /**
     * @brief     Initialize a new MVW geometry buffer
     *
     * @param[in] id     Identifier for this buffer
     */
    mvw_buffer(const std::string &id);

    inline bool render_quad() const
    { return render_quad_; }

    inline void render_quad(bool new_render_quad)
    { render_quad_ = new_render_quad; }
};

#endif /* _MVW_BUFFER_HPP_ */
