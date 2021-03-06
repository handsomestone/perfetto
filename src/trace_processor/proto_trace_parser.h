/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef SRC_TRACE_PROCESSOR_PROTO_TRACE_PARSER_H_
#define SRC_TRACE_PROCESSOR_PROTO_TRACE_PARSER_H_

#include <stdint.h>
#include <memory>

#include "src/trace_processor/trace_blob_view.h"

namespace perfetto {
namespace trace_processor {

class TraceProcessorContext;

class ProtoTraceParser {
 public:
  explicit ProtoTraceParser(TraceProcessorContext*);
  virtual ~ProtoTraceParser();

  // virtual for testing.
  virtual void ParseTracePacket(TraceBlobView);
  virtual void ParseFtracePacket(uint32_t cpu,
                                 uint64_t timestamp,
                                 TraceBlobView);
  void ParseProcessTree(TraceBlobView);
  void ParseSchedSwitch(uint32_t cpu, uint64_t timestamp, TraceBlobView);
  void ParseCpuFreq(uint64_t timestamp, TraceBlobView);
  void ParseThread(TraceBlobView);
  void ParseProcess(TraceBlobView);

 private:
  TraceProcessorContext* context_;
};

}  // namespace trace_processor
}  // namespace perfetto

#endif  // SRC_TRACE_PROCESSOR_PROTO_TRACE_PARSER_H_
