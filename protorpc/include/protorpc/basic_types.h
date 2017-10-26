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

#ifndef PROTORPC_INCLUDE_PROTORPC_BASIC_TYPES_H_
#define PROTORPC_INCLUDE_PROTORPC_BASIC_TYPES_H_

#include <stdint.h>

#include <functional>
#include <memory>

namespace google {
namespace protobuf {
class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace perfetto {
namespace protorpc {

class ServiceProxy;
class MethodInvocationReplyBase;

using ProtoMessage = ::google::protobuf::MessageLite;
using ServiceID = uint64_t;
using MethodID = uint64_t;
using ClientID = uint64_t;
using RequestID = uint64_t;
using MethodInvokeCallback = std::function<void(MethodInvocationReplyBase)>;

}  // namespace protorpc
}  // namespace perfetto

#endif  // PROTORPC_INCLUDE_PROTORPC_BASIC_TYPES_H_
