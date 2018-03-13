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

#include "src/traced/probes/filesystem/inode_file_data_source.h"

#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <queue>

#include "include/perfetto/ftrace_reader/ftrace_controller.h"
#include "perfetto/base/logging.h"
#include "perfetto/tracing/core/trace_packet.h"

#include "perfetto/trace/trace_packet.pbzero.h"

namespace perfetto {

using BlockDeviceID = decltype(stat::st_dev);

void CreateDeviceToInodeMap(
    const std::string& root_directory,
    std::map<BlockDeviceID, std::map<Inode, InodeMapValue>>* block_device_map) {
  std::queue<std::string> queue;
  queue.push(root_directory);
  while (!queue.empty()) {
    struct dirent* entry;
    std::string filepath = queue.front();
    queue.pop();
    DIR* dir = opendir(filepath.c_str());
    filepath += "/";
    if (dir == nullptr)
      continue;
    while ((entry = readdir(dir)) != nullptr) {
      std::string filename = entry->d_name;
      if (filename == "." || filename == "..")
        continue;
      Inode inode_number = entry->d_ino;
      struct stat buf;
      if (lstat(filepath.c_str(), &buf) != 0)
        continue;
      BlockDeviceID block_device_id = buf.st_dev;
      std::map<Inode, InodeMapValue>& inode_map =
          (*block_device_map)[block_device_id];
      // Default
      protos::pbzero::InodeFileMap_Entry_Type type =
          protos::pbzero::InodeFileMap_Entry_Type_UNKNOWN;
      // Readdir and stat not guaranteed to have directory info for all systems
      if (entry->d_type == DT_DIR || S_ISDIR(buf.st_mode)) {
        // Continue iterating through files if current entry is a directory
        queue.push(filepath + filename);
        type = protos::pbzero::InodeFileMap_Entry_Type_DIRECTORY;
      } else if (entry->d_type == DT_REG || S_ISREG(buf.st_mode)) {
        type = protos::pbzero::InodeFileMap_Entry_Type_FILE;
      }
      inode_map[inode_number].setType(type);
      inode_map[inode_number].addPath(filepath + filename);
    }
    closedir(dir);
  }
}

InodeFileMapDataSource::InodeFileMapDataSource(
    std::map<BlockDeviceID, std::map<Inode, InodeMapValue>>* file_system_inodes,
    std::unique_ptr<TraceWriter> writer)
    : file_system_inodes_(file_system_inodes), writer_(std::move(writer)) {}

InodeFileMapDataSource::~InodeFileMapDataSource() = default;

void InodeFileMapDataSource::WriteInodes(const FtraceMetadata& metadata) {
  PERFETTO_LOG("Write Inodes start");

  if (mount_points_.empty()) {
    mount_points_ = ParseMounts();
  }
  // Convert FtraceMetadata into format for InodeFileMap proto
  auto inode_to_device = metadata.inode_to_device;
  std::map<BlockDeviceID, std::set<Inode>> inode_file_maps;
  for (const auto& inode : inode_to_device) {
    BlockDeviceID block_device_id = inode.first;
    Inode inode_number = inode.second;
    inode_file_maps[block_device_id].emplace(inode_number);
  }
  // Write a TracePacket with an InodeFileMap proto for each block device id
  for (const auto& inode_file_map_data : inode_file_maps) {
    auto trace_packet = writer_->NewTracePacket();
    auto inode_file_map = trace_packet->set_inode_file_map();
    // Add block device id
    BlockDeviceID block_device_id = inode_file_map_data.first;
    inode_file_map->set_block_device_id(block_device_id);
    // Add mount points
    std::pair<std::multimap<BlockDeviceID, std::string>::iterator,
              std::multimap<BlockDeviceID, std::string>::iterator>
        range;
    range = mount_points_.equal_range(block_device_id);
    for (std::multimap<BlockDeviceID, std::string>::iterator it = range.first;
         it != range.second; ++it) {
      inode_file_map->add_mount_points(it->second.c_str());
      PERFETTO_LOG("Mount point=%s", it->second.c_str());
    }
    // Add entries for each inode number
    std::set<Inode> inode_numbers = inode_file_map_data.second;
    for (const auto& inode_number : inode_numbers) {
      PERFETTO_LOG("Inode number=%" PRIu64, inode_number);
      auto* entry = inode_file_map->add_entries();
      entry->set_inode_number(inode_number);
      auto block_device_map = file_system_inodes_->find(block_device_id);
      if (block_device_map != file_system_inodes_->end()) {
        auto inode_map = block_device_map->second.find(inode_number);
        if (inode_map != block_device_map->second.end()) {
          entry->set_type(inode_map->second.getType());
          for (const auto& path : inode_map->second.getPaths())
            entry->add_paths(path.c_str());
        }
      }
    }
    trace_packet->Finalize();
  }
}

}  // namespace perfetto
