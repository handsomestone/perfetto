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

#ifndef SRC_TRACING_TEST_MOCK_PRODUCER_H_
#define SRC_TRACING_TEST_MOCK_PRODUCER_H_

#include <map>
#include <memory>
#include <string>

#include "gmock/gmock.h"
#include "perfetto/tracing/core/producer.h"
#include "perfetto/tracing/core/service.h"
#include "perfetto/tracing/core/trace_writer.h"

namespace perfetto {

namespace base {
class TestTaskRunner;
}

class MockProducer : public Producer {
 public:
  struct EnabledDataSource {
    DataSourceInstanceID id;
    BufferID target_buffer;
  };

  explicit MockProducer(base::TestTaskRunner*);
  ~MockProducer() override;

  void Connect(Service* svc, const std::string& producer_name, uid_t uid = 42);
  void RegisterDataSource(const std::string& name);
  void UnregisterDataSource(const std::string& name);
  void WaitForTracingEnabled();
  void WaitForTracingDisabled();
  void WaitForDataSourceStart(const std::string& name);
  void WaitForDataSourceStop(const std::string& name);
  std::unique_ptr<TraceWriter> CreateTraceWriter(
      const std::string& data_source_name);
  void WaitForFlush(TraceWriter* writer_to_flush);

  Service::ProducerEndpoint* endpoint() { return service_endpoint_.get(); }

  // Producer implementation.
  MOCK_METHOD0(OnConnect, void());
  MOCK_METHOD0(OnDisconnect, void());
  MOCK_METHOD2(CreateDataSourceInstance,
               void(DataSourceInstanceID, const DataSourceConfig&));
  MOCK_METHOD1(TearDownDataSourceInstance, void(DataSourceInstanceID));
  MOCK_METHOD0(OnTracingStart, void());
  MOCK_METHOD0(OnTracingStop, void());
  MOCK_METHOD3(Flush,
               void(FlushRequestID, const DataSourceInstanceID*, size_t));

 private:
  base::TestTaskRunner* const task_runner_;
  std::string producer_name_;
  std::unique_ptr<Service::ProducerEndpoint> service_endpoint_;
  std::map<std::string, EnabledDataSource> data_source_instances_;
};

}  // namespace perfetto

#endif  // SRC_TRACING_TEST_MOCK_PRODUCER_H_
