/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*******************************************************************************
 * AUTOGENERATED - DO NOT EDIT
 *******************************************************************************
 * This file has been generated from the protobuf message
 * protos/tracing_service/data_source_config.proto
 * by
 * ../../tools/proto_to_cpp/proto_to_cpp.cc.
 * If you need to make changes here, change the .proto file and then run
 * ./tools/gen_tracing_cpp_headers_from_protos.py
 */

#include "include/perfetto/tracing/core/data_source_config.h"

#include "protos/tracing_service/data_source_config.pb.h"

namespace perfetto {

DataSourceConfig::DataSourceConfig() = default;
DataSourceConfig::~DataSourceConfig() = default;
DataSourceConfig::DataSourceConfig(const DataSourceConfig&) = default;
DataSourceConfig& DataSourceConfig::operator=(const DataSourceConfig&) =
    default;
DataSourceConfig::DataSourceConfig(DataSourceConfig&&) noexcept = default;
DataSourceConfig& DataSourceConfig::operator=(DataSourceConfig&&) = default;

void DataSourceConfig::FromProto(
    const perfetto::protos::DataSourceConfig& proto) {
  static_assert(sizeof(name_) == sizeof(proto.name()), "size mismatch");
  name_ = static_cast<decltype(name_)>(proto.name());

  static_assert(sizeof(target_buffer_) == sizeof(proto.target_buffer()),
                "size mismatch");
  target_buffer_ = static_cast<decltype(target_buffer_)>(proto.target_buffer());

  static_assert(
      sizeof(trace_category_filters_) == sizeof(proto.trace_category_filters()),
      "size mismatch");
  trace_category_filters_ = static_cast<decltype(trace_category_filters_)>(
      proto.trace_category_filters());

  ftrace_config_.FromProto(proto.ftrace_config());
  unknown_fields_ = proto.unknown_fields();
}

void DataSourceConfig::ToProto(
    perfetto::protos::DataSourceConfig* proto) const {
  proto->Clear();

  static_assert(sizeof(name_) == sizeof(proto->name()), "size mismatch");
  proto->set_name(static_cast<decltype(proto->name())>(name_));

  static_assert(sizeof(target_buffer_) == sizeof(proto->target_buffer()),
                "size mismatch");
  proto->set_target_buffer(
      static_cast<decltype(proto->target_buffer())>(target_buffer_));

  static_assert(sizeof(trace_category_filters_) ==
                    sizeof(proto->trace_category_filters()),
                "size mismatch");
  proto->set_trace_category_filters(
      static_cast<decltype(proto->trace_category_filters())>(
          trace_category_filters_));

  ftrace_config_.ToProto(proto->mutable_ftrace_config());
  *(proto->mutable_unknown_fields()) = unknown_fields_;
}

DataSourceConfig::FtraceConfig::FtraceConfig() = default;
DataSourceConfig::FtraceConfig::~FtraceConfig() = default;
DataSourceConfig::FtraceConfig::FtraceConfig(
    const DataSourceConfig::FtraceConfig&) = default;
DataSourceConfig::FtraceConfig& DataSourceConfig::FtraceConfig::operator=(
    const DataSourceConfig::FtraceConfig&) = default;
DataSourceConfig::FtraceConfig::FtraceConfig(
    DataSourceConfig::FtraceConfig&&) noexcept = default;
DataSourceConfig::FtraceConfig& DataSourceConfig::FtraceConfig::operator=(
    DataSourceConfig::FtraceConfig&&) = default;

void DataSourceConfig::FtraceConfig::FromProto(
    const perfetto::protos::DataSourceConfig_FtraceConfig& proto) {
  event_names_.clear();
  for (const auto& field : proto.event_names()) {
    event_names_.emplace_back();
    static_assert(sizeof(event_names_.back()) == sizeof(proto.event_names(0)),
                  "size mismatch");
    event_names_.back() =
        static_cast<decltype(event_names_)::value_type>(field);
  }

  atrace_categories_.clear();
  for (const auto& field : proto.atrace_categories()) {
    atrace_categories_.emplace_back();
    static_assert(
        sizeof(atrace_categories_.back()) == sizeof(proto.atrace_categories(0)),
        "size mismatch");
    atrace_categories_.back() =
        static_cast<decltype(atrace_categories_)::value_type>(field);
  }

  atrace_apps_.clear();
  for (const auto& field : proto.atrace_apps()) {
    atrace_apps_.emplace_back();
    static_assert(sizeof(atrace_apps_.back()) == sizeof(proto.atrace_apps(0)),
                  "size mismatch");
    atrace_apps_.back() =
        static_cast<decltype(atrace_apps_)::value_type>(field);
  }
  unknown_fields_ = proto.unknown_fields();
}

void DataSourceConfig::FtraceConfig::ToProto(
    perfetto::protos::DataSourceConfig_FtraceConfig* proto) const {
  proto->Clear();

  for (const auto& it : event_names_) {
    auto* entry = proto->add_event_names();
    static_assert(sizeof(it) == sizeof(proto->event_names(0)), "size mismatch");
    *entry = static_cast<decltype(proto->event_names(0))>(it);
  }

  for (const auto& it : atrace_categories_) {
    auto* entry = proto->add_atrace_categories();
    static_assert(sizeof(it) == sizeof(proto->atrace_categories(0)),
                  "size mismatch");
    *entry = static_cast<decltype(proto->atrace_categories(0))>(it);
  }

  for (const auto& it : atrace_apps_) {
    auto* entry = proto->add_atrace_apps();
    static_assert(sizeof(it) == sizeof(proto->atrace_apps(0)), "size mismatch");
    *entry = static_cast<decltype(proto->atrace_apps(0))>(it);
  }
  *(proto->mutable_unknown_fields()) = unknown_fields_;
}

}  // namespace perfetto
