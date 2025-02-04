// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <openvino/frontend/manager.hpp>
#include <openvino/openvino.hpp>

#include "gtest/gtest.h"
#include "tf_utils.hpp"
#include "utils.hpp"

namespace {
std::shared_ptr<ov::Model> convert_model(const std::string& model_path) {
    ov::frontend::FrontEndManager fem;
    auto front_end = fem.load_by_framework(TF_FE);
    if (!front_end) {
        throw "TensorFlow Frontend is not initialized";
    }
    auto model_filename = FrontEndTestUtils::make_model_path(std::string(TEST_TENSORFLOW_MODELS_DIRNAME) + model_path);
    auto input_model = front_end->load(model_filename);
    if (!input_model) {
        throw "Input model is not read";
    }
    auto model = front_end->convert(input_model);
    if (!model) {
        throw "Model is not converted";
    }

    return model;
}
}  // namespace

class CompileModelsTests : public ::testing::Test {};

TEST_F(CompileModelsTests, NgramCompilation) {
    ov::Core core;
    auto model = convert_model("model_ngram/model_ngram.pbtxt");
    ov::CompiledModel compiled_model = core.compile_model(model, "CPU");
    const auto runtime_model = compiled_model.get_runtime_model();

    EXPECT_EQ(runtime_model->get_ordered_ops().size(), 4);
    EXPECT_EQ(runtime_model->get_parameters().size(), 2);
    EXPECT_EQ(runtime_model->get_results().size(), 1);
}