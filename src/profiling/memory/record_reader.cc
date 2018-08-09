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

#include "src/profiling/memory/record_reader.h"

#include "perfetto/base/logging.h"

#include <unistd.h>
#include <algorithm>
#include <limits>

namespace perfetto {

RecordReader::ReceiveBuffer RecordReader::BeginReceive() {
  if (read_idx_ < sizeof(record_size_))
    return {&record_size_, sizeof(record_size_) - read_idx_};
  return {buf_.get(), (record_size_ + sizeof(record_size_) - read_idx_)};
}

bool RecordReader::EndReceive(size_t recv_size, Record* record) {
  read_idx_ += recv_size;
  if (read_idx_ == sizeof(record_size_))
    buf_.reset(new uint8_t[record_size_]);

  if (read_idx_ == record_size_ + sizeof(record_size_)) {
    std::unique_ptr<uint8_t[]> ret = std::move(buf_);
    size_t size = record_size_;
    Reset();
    record->data = std::move(ret);
    record->size = size;
    return true;
  }
  return false;
}

void RecordReader::Reset() {
  read_idx_ = 0;
  record_size_ = 0;
}

}  // namespace perfetto
