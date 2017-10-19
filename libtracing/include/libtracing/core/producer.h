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

#ifndef LIBTRACING_INCLUDE_LIBTRACING_CORE_PRODUCER_H_
#define LIBTRACING_INCLUDE_LIBTRACING_CORE_PRODUCER_H_

#include "libtracing/core/basic_types.h"

namespace perfetto {

class DataSourceConfig;

// The callback interface that concrete Producer(s) the libtracing clients
// have to implement in order to register and interact with the service.
//
// Exposed to:
//   The transport layer (e.g., unix_transport/unix_service_connection) when
//   establishing a connection to the Service.
//
// Subclassed by:
//   The Producer code in the libtracing client e.g., the ftrace reader process.
class Producer {
 public:
  virtual ~Producer() {}

  virtual void OnConnect() = 0;

  virtual void CreateDataSourceInstance(DataSourceInstanceID,
                                        const DataSourceConfig&) = 0;

  virtual void TearDownDataSourceInstance(DataSourceInstanceID) = 0;
};

}  // namespace perfetto

#endif  // LIBTRACING_INCLUDE_LIBTRACING_CORE_PRODUCER_H_
