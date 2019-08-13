#ifndef _DATA_INPUT_HPP_
#define _DATA_INPUT_HPP_

#include <array>
#include <vector>

#include <shadertoy/inputs/basic_input.hpp>

enum data_input_state
{
    dis_gpu_dirty,
    dis_gpu_uptodate,
};

struct data_input : public shadertoy::inputs::basic_input
{
    std::vector<float> data;
    std::array<uint32_t, 3> dims;

    data_input_state state;

    data_input();

    data_input(const data_input &) = delete;
    data_input &operator=(const data_input &) = delete;

  protected:
    GLenum load_input() override;
    void reset_input() override;
    shadertoy::backends::gx::texture *use_input() override;

  private:
    std::unique_ptr<shadertoy::backends::gx::texture> tex_;
};

#endif /* _DATA_INPUT_HPP_ */
