// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "decoder_argdef.hpp"

#include "decoder_proto.hpp"
#include "op_def.pb.h"
#include "openvino/frontend/tensorflow/node_context.hpp"
#include "openvino/frontend/tensorflow/special_types.hpp"
#include "types.pb.h"

namespace ov {
namespace frontend {
namespace tensorflow {

size_t DecoderArgDef::get_input_size() const {
    FRONT_END_GENERAL_CHECK(m_op_type == "input_arg" || m_op_type == "output_arg",
                            "[TensorFlow Frontend] Internal error: Incorrect use of DecoderArgDef class.");
    return m_op_type == "input_arg" ? 0 : 1;
}

const std::string& DecoderArgDef::get_op_type() const {
    FRONT_END_GENERAL_CHECK(m_op_type == "input_arg" || m_op_type == "output_arg",
                            "[TensorFlow Frontend] Internal error: Incorrect use of DecoderArgDef class.");
    return m_op_type;
}

const std::string& DecoderArgDef::get_op_name() const {
    return m_arg_def->name();
}

void DecoderArgDef::get_input_node(size_t input_port_idx,
                                   std::string& producer_name,
                                   size_t& producer_output_port_index) const {
    // Body graph nodes may have two colons `:`, for example,
    // producer_name:z:2 means that producer operation name is `producer_name`
    // and output port is 2
    FRONT_END_GENERAL_CHECK(m_op_type == "output_arg",
                            "[TensorFlow Frontend] Internal error: get_input_node is supported only for output_arg.");
    parse_producer_name(m_producer_name, producer_name, producer_output_port_index, {});
}

void DecoderArgDef::get_input_node(size_t input_port_idx,
                                   std::string& producer_name,
                                   size_t& producer_output_port_index,
                                   const OpTypeByName& op_type_by_name) const {
    FRONT_END_GENERAL_CHECK(m_op_type == "output_arg",
                            "[TensorFlow Frontend] Internal error: get_input_node is supported only for output_arg.");
    parse_producer_name(m_producer_name, producer_name, producer_output_port_index, op_type_by_name);
}

ov::Any DecoderArgDef::get_attribute(const std::string& name) const {
    FRONT_END_GENERAL_CHECK(name == "type",
                            "[TensorFlow Frontend] Internal error: DecoderArgDef supports only `type` attribute.");
    return get_ov_type(m_arg_def->type());
}

}  // namespace tensorflow
}  // namespace frontend
}  // namespace ov
