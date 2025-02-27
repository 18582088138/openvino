// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "test_utils.h"

#include <intel_gpu/primitives/input_layout.hpp>
#include <intel_gpu/primitives/reshape.hpp>
#include <intel_gpu/primitives/data.hpp>

#include "reshape_inst.h"

#include "program_wrapper.h"

#include <cmath>
#include <algorithm>

using namespace cldnn;
using namespace ::tests;

namespace shape_infer_tests {

struct reshape_test_params {
    layout in_layout;
    layout pattern_layout;
    std::vector<int64_t> pattern_data;
    ov::PartialShape output_partial_shape;
    bool special_zero;
    layout expected_layout;
};

class reshape_test_two_inputs : public testing::TestWithParam<reshape_test_params> { };
TEST_P(reshape_test_two_inputs, shape_infer) {
    auto p = GetParam();

    auto& engine = get_test_engine();

    auto input_prim = std::make_shared<input_layout>("input", p.in_layout);
    auto pattern_prim = std::make_shared<input_layout>("pattern", p.pattern_layout);
    auto reshape_prim = std::make_shared<reshape>("output", "input", "pattern", p.special_zero, p.output_partial_shape);

    cldnn::program prog(engine);

    auto pattern_mem = engine.allocate_memory(p.pattern_layout);
    set_values(pattern_mem, p.pattern_data);

    auto& input_node = prog.get_or_create(input_prim);
    auto& pattern_node = prog.get_or_create(pattern_prim);
    auto& reshape_node = prog.get_or_create(reshape_prim);
    program_wrapper::add_connection(prog, input_node, reshape_node);
    program_wrapper::add_connection(prog, pattern_node, reshape_node);
    auto params = reshape_node.get_kernel_impl_params();

    auto res_wo_data = reshape_inst::calc_output_layouts<ov::PartialShape>(reshape_node, *params);

    params->memory_deps = {{1, pattern_mem}};
    auto res_w_data = reshape_inst::calc_output_layouts<ov::PartialShape>(reshape_node, *params);

    layout expected_layout_wo_data{p.output_partial_shape, p.expected_layout.data_type, p.expected_layout.format};
    ASSERT_EQ(res_wo_data.size(), 1);
    ASSERT_EQ(res_wo_data[0], expected_layout_wo_data);

    ASSERT_EQ(res_w_data.size(), 1);
    ASSERT_EQ(res_w_data[0], p.expected_layout);
}

INSTANTIATE_TEST_SUITE_P(smoke, reshape_test_two_inputs,
    testing::ValuesIn(std::vector<reshape_test_params>{
        {
            layout{ov::PartialShape{1, 128, 1024}, data_types::f32, format::bfyx},
            layout{ov::PartialShape{4}, data_types::i64, format::bfyx}, {2, 64, 1, 1024}, ov::PartialShape::dynamic(4), false,
            layout{ov::PartialShape{2, 64, 1, 1024}, data_types::f32, format::bfyx}
        },
        {
            layout{ov::PartialShape{1, 128, 1024}, data_types::f32, format::bfyx},
            layout{ov::PartialShape{3}, data_types::i64, format::bfyx}, {2, 64, -1}, ov::PartialShape::dynamic(3), false,
            layout{ov::PartialShape{2, 64, 1024}, data_types::f32, format::bfyx}
        },
        {
            layout{ov::PartialShape{1, 384, 1024}, data_types::f32, format::bfyx},
            layout{ov::PartialShape{4}, data_types::i64, format::bfyx}, {0, 0, 16, 64}, ov::PartialShape::dynamic(4), true,
            layout{ov::PartialShape{1, 384, 16, 64}, data_types::f32, format::bfyx}
        },
    }));

class reshape_test_single_input : public testing::TestWithParam<reshape_test_params> { };
TEST_P(reshape_test_single_input, shape_infer) {
    auto p = GetParam();

    auto& engine = get_test_engine();

    auto input_prim = std::make_shared<input_layout>("input", p.in_layout);
    auto reshape_prim = std::make_shared<reshape>("output", "input", p.special_zero, p.pattern_data, p.output_partial_shape);

    cldnn::program prog(engine);

    auto& input_node = prog.get_or_create(input_prim);
    auto& reshape_node = prog.get_or_create(reshape_prim);
    program_wrapper::add_connection(prog, input_node, reshape_node);
    auto res = reshape_inst::calc_output_layouts<ov::PartialShape>(reshape_node, *reshape_node.get_kernel_impl_params());

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(res[0], p.expected_layout);
}

INSTANTIATE_TEST_SUITE_P(smoke, reshape_test_single_input,
    testing::ValuesIn(std::vector<reshape_test_params>{
        {
            layout{ov::PartialShape{1, 128, 1024}, data_types::f32, format::bfyx},
            layout{ov::PartialShape{4}, data_types::i64, format::bfyx}, {2, 64, 1, 1024}, ov::PartialShape::dynamic(), false,
            layout{ov::PartialShape{2, 64, 1, 1024}, data_types::f32, format::bfyx}
        },
        {
            layout{ov::PartialShape{1, 128, 1024}, data_types::f32, format::bfyx},
            layout{ov::PartialShape{3}, data_types::i64, format::bfyx}, {2, 64, -1}, ov::PartialShape::dynamic(), false,
            layout{ov::PartialShape{2, 64, 1024}, data_types::f32, format::bfyx}
        },
        {
            layout{ov::PartialShape{1, 384, 1024}, data_types::f32, format::bfyx},
            layout{ov::PartialShape{4}, data_types::i64, format::bfyx}, {0, 0, 16, 64}, ov::PartialShape::dynamic(), true,
            layout{ov::PartialShape{1, 384, 16, 64}, data_types::f32, format::bfyx}
        },
        {
            layout{ov::PartialShape{1, 384, 1024}, data_types::f32, format::bfyx},
            layout{ov::PartialShape{4}, data_types::i64, format::bfyx}, {}, ov::PartialShape::dynamic(2), true,
            layout{ov::PartialShape::dynamic(2), data_types::f32, format::bfyx}
        },
    }));

}  // shape_infer_tests
