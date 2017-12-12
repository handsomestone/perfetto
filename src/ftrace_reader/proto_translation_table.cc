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

#include "proto_translation_table.h"

#include <algorithm>

#include "event_info.h"
#include "ftrace_procfs.h"
#include "perfetto/ftrace_reader/format_parser.h"
#include "perfetto/ftrace_reader/ftrace_to_proto.h"

#include "protos/ftrace/ftrace_event_bundle.pbzero.h"

namespace perfetto {

namespace {

const std::vector<Event> BuildEventsVector(const std::vector<Event>& events) {
  size_t largest_id = 0;
  for (const Event& event : events) {
    if (event.ftrace_event_id > largest_id)
      largest_id = event.ftrace_event_id;
  }
  std::vector<Event> events_by_id;
  events_by_id.resize(largest_id + 1);
  for (const Event& event : events) {
    events_by_id[event.ftrace_event_id] = event;
  }
  events_by_id.shrink_to_fit();
  return events_by_id;
}

bool MergeFieldInfo(Field* field, const FtraceEvent::Field& ftrace_field) {
  PERFETTO_DCHECK(field->ftrace_name);
  PERFETTO_DCHECK(field->proto_field_id);
  PERFETTO_DCHECK(field->proto_field_type);
  PERFETTO_DCHECK(!field->ftrace_offset);
  PERFETTO_DCHECK(!field->ftrace_size);
  // TODO(hjd): Re-instate this after we decide this at runtime.
  // PERFETTO_DCHECK(!field.ftrace_type);

  // TODO(hjd): Set field.ftrace_type here.
  field->ftrace_offset = ftrace_field.offset;
  field->ftrace_size = ftrace_field.size;

  bool can_consume = SetTranslationStrategy(
      field->ftrace_type, field->proto_field_type, &field->strategy);

  return can_consume;
}

uint16_t MergeFields(std::vector<Field>* fields,
                     const std::vector<FtraceEvent::Field>& ftrace_fields) {
  uint16_t max_offset = 0;

  auto field = fields->begin();
  while (field != fields->end()) {
    bool success = false;
    for (const FtraceEvent::Field& ftrace_field : ftrace_fields) {
      if (GetNameFromTypeAndName(ftrace_field.type_and_name) !=
          field->ftrace_name)
        continue;

      success = MergeFieldInfo(&*field, ftrace_field);

      uint16_t max_for_field = field->ftrace_offset + field->ftrace_size;
      max_offset = std::max<uint16_t>(max_offset, max_for_field);

      break;
    }
    if (success) {
      ++field;
    } else {
      fields->erase(field);
    }
  }
  return max_offset;
}

}  // namespace

// static
std::unique_ptr<ProtoTranslationTable> ProtoTranslationTable::Create(
    const FtraceProcfs* ftrace_procfs,
    std::vector<Event> events,
    std::vector<Field> common_fields) {
  bool common_fields_processed = false;
  uint16_t common_fields_end = 0;

  for (Event& event : events) {
    PERFETTO_DCHECK(event.name);
    PERFETTO_DCHECK(event.group);
    PERFETTO_DCHECK(event.proto_field_id);
    PERFETTO_DCHECK(!event.ftrace_event_id);

    std::string contents =
        ftrace_procfs->ReadEventFormat(event.group, event.name);
    FtraceEvent ftrace_event;
    if (contents == "" || !ParseFtraceEvent(contents, &ftrace_event)) {
      continue;
    }

    event.ftrace_event_id = ftrace_event.id;

    if (!common_fields_processed) {
      common_fields_end =
          MergeFields(&common_fields, ftrace_event.common_fields);
      common_fields_processed = true;
    }

    uint16_t fields_end = MergeFields(&event.fields, ftrace_event.fields);

    event.size = std::max<uint16_t>(fields_end, common_fields_end);
  }

  events.erase(std::remove_if(events.begin(), events.end(),
                              [](const Event& event) {
                                return event.proto_field_id == 0 ||
                                       event.ftrace_event_id == 0;
                              }),
               events.end());

  auto table = std::unique_ptr<ProtoTranslationTable>(
      new ProtoTranslationTable(std::move(events), std::move(common_fields)));
  return table;
}

ProtoTranslationTable::ProtoTranslationTable(const std::vector<Event>& events,
                                             std::vector<Field> common_fields)
    : events_(BuildEventsVector(events)),
      largest_id_(events_.size() - 1),
      common_fields_(std::move(common_fields)) {
  for (const Event& event : events) {
    name_to_event_[event.name] = &events_.at(event.ftrace_event_id);
  }
}

ProtoTranslationTable::~ProtoTranslationTable() = default;

}  // namespace perfetto
