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

#ifndef INCLUDE_PERFETTO_TRACE_PROCESSOR_BLOB_READER_H_
#define INCLUDE_PERFETTO_TRACE_PROCESSOR_BLOB_READER_H_

#include <functional>

namespace perfetto {
namespace trace_processor {

class BlobReader {
 public:
  // class Delegate {
  //  public:
  //   virtual void OnRead(uint64_t offset,
  //                       const uint8_t* data,
  //                       size_t length) = 0;
  // };

  virtual ~BlobReader();

  using ReadCallback =
      std::function<void(uint32_t /*offset*/, const uint8_t* /*data*/, size_t)>;
  virtual void Read(uint32_t offset, size_t max_size, ReadCallback) = 0;
};

}  // namespace trace_processor
}  // namespace perfetto

#endif  // INCLUDE_PERFETTO_TRACE_PROCESSOR_BLOB_READER_H_
