// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include <memory>
#include <openvino/opsets/opset10.hpp>
#include <string>
#include <transformations/common_optimizations/nonzero_fusion.hpp>

#include "common_test_utils/ngraph_test_utils.hpp"

using namespace testing;

enum NonZeroType { I32, I64, NONE };

struct NonZeroFusionBuilder {
    NonZeroFusionBuilder() = default;
    NonZeroFusionBuilder(const std::vector<NonZeroType>& props) : branch_props(props) {}

    std::shared_ptr<ov::Model> getOriginal() {
        const auto input = std::make_shared<ov::opset10::Parameter>(ov::element::f32, ov::PartialShape::dynamic(4));
        ov::NodeVector results;
        for (size_t i = 0; i < branch_props.size(); ++i) {
            std::shared_ptr<ov::Node> nonzero;
            switch (branch_props[i]) {
            case NonZeroType::I32:
                nonzero = std::make_shared<ov::opset10::NonZero>(input, ov::element::i32);
                break;
            case NonZeroType::I64:
                nonzero = std::make_shared<ov::opset10::NonZero>(input, ov::element::i64);
                break;
            default:
                nonzero = input;
                break;
            }
            auto last_node = std::make_shared<ov::opset10::Relu>(nonzero);
            last_node->set_friendly_name("last_node_" + std::to_string(i));
            results.push_back(last_node);
        }
        return std::make_shared<ov::Model>(results, ov::ParameterVector{input});
    };

    std::shared_ptr<ov::Model> getReference() {
        const auto input = std::make_shared<ov::opset10::Parameter>(ov::element::f32, ov::PartialShape::dynamic(4));

        std::shared_ptr<ov::Node> i32_node;
        std::shared_ptr<ov::Node> i64_node;
        ov::NodeVector results;
        for (size_t i = 0; i < branch_props.size(); ++i) {
            std::shared_ptr<ov::Node> nonzero;
            if (branch_props[i] == NonZeroType::I32) {
                nonzero = i32_node ? i32_node : std::make_shared<ov::opset10::NonZero>(input, ov::element::i32);
                if (!i32_node)
                    i32_node = nonzero;
            } else if (branch_props[i] == NonZeroType::I64) {
                nonzero = i64_node ? i64_node : std::make_shared<ov::opset10::NonZero>(input, ov::element::i64);
                if (!i64_node)
                    i64_node = nonzero;
            } else {
                nonzero = input;
            }
            auto last_node = std::make_shared<ov::opset10::Relu>(nonzero);
            last_node->set_friendly_name("last_node_" + std::to_string(i));
            results.push_back(last_node);
        }
        return std::make_shared<ov::Model>(results, ov::ParameterVector{input});
    }

    std::vector<NonZeroType> branch_props;
};

class NonZeroFusionTests : public testing::WithParamInterface<std::vector<NonZeroType>>, public TransformationTestsF {
public:
    NonZeroFusionTests() : TransformationTestsF() {
        comparator.enable(FunctionsComparator::CONSUMERS_COUNT);
    }

    static std::string getTestCaseName(testing::TestParamInfo<std::vector<NonZeroType>> obj) {
        const std::vector<NonZeroType> testValues = obj.param;
        std::ostringstream result;
        result << "branch_props_{";
        for (const auto& value : testValues) {
            switch (value) {
            case NonZeroType::I32:
                result << "nonzero_i32,";
                break;
            case NonZeroType::I64:
                result << "nonzero_i64,";
                break;
            default:
                result << "wo_nonzero,";
                break;
            }
        }
        result << "}";
        return result.str();
    }

protected:
    void SetUp() override {
        TransformationTestsF::SetUp();
        const auto branch_props = GetParam();
        builder = NonZeroFusionBuilder(branch_props);
        manager.register_pass<ov::pass::NonZeroFusion>();
    }

    NonZeroFusionBuilder builder;
};

TEST_P(NonZeroFusionTests, NonZeroFusion) {
    model = builder.getOriginal();
    model_ref = builder.getReference();
}

namespace NonZeroFusionTestsInstantiation {
std::vector<std::vector<NonZeroType>> test_params{std::vector<NonZeroType>(5, I32),
                                                  std::vector<NonZeroType>(5, I64),
                                                  std::vector<NonZeroType>(2, NONE),
                                                  {I32, I64, I32, I64, I32},
                                                  {I32, I64, NONE, I64, I32},
                                                  {NONE, I64, NONE, I64, I32}};

INSTANTIATE_TEST_SUITE_P(TransformationTestsF,
                         NonZeroFusionTests,
                         ::testing::ValuesIn(test_params),
                         NonZeroFusionTests::getTestCaseName);

}  // namespace NonZeroFusionTestsInstantiation
