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

#ifndef SRC_TRACED_PROBES_FILESYSTEM_PREFIX_FINDER_H_
#define SRC_TRACED_PROBES_FILESYSTEM_PREFIX_FINDER_H_

#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "perfetto/base/logging.h"

namespace perfetto {

// PrefixFinder allows to find prefixes for filenames that ensure that
// they can be found again within a provided limit of steps when searching
// within that prefix in the same order.
//
// For instance, let the limit be 4 and the filesystem be.
// /a/1
// /a/2
// /a/3
// /b/4
// /b/5
//
// The prefix for /a/1, /a/2 and /a/3/ is /, the one for /b/4 and /b/5 is /b/.
class PrefixFinder {
 public:
  // Opaque placeholder for a prefix that can be turned into a string
  // using ToString.
  // Can not be constructed outside of PrefixFinder.
  class Node {
   public:
    friend class PrefixFinder;

    Node(const Node& that) = delete;
    Node& operator=(const Node&) = delete;

    // Return string representation of prefix, e.g. /foo/bar.
    // Does not enclude a trailing /.
    std::string ToString();

   private:
    Node(std::string name, Node* parent) : name_(name), parent_(parent) {}

    std::string name_;
    std::map<std::string, std::unique_ptr<Node>> children_;
    Node* parent_;
  };

  PrefixFinder(size_t limit);

  // Add path to prefix mapping.
  // This *HAS* to be called in DFS order.
  // Must not be called after Finalize.
  // Must be called before GetPrefix(path) for the same path.
  void AddPath(std::string path);

  // Return identifier for prefix. Ownership remains with the PrefixFinder.
  // Must not be before after Finalize.
  // Must be called after AddPath(path) for the same path.
  Node* GetPrefix(std::string path);

  // Call this after the last AddPath and before the first GetPrefix.
  void Finalize();

 private:
  void Flush(size_t i);
  void InsertPrefix(size_t len);
  const size_t limit_;
  // (path element, count) tuples for last path seen.
  std::vector<std::pair<std::string, size_t>> state_{{"", 0}};
  Node root_{"", nullptr};
#if PERFETTO_DCHECK_IS_ON()
  bool finalized_ = false;
#endif
};

}  // namespace perfetto
#endif  // SRC_TRACED_PROBES_FILESYSTEM_PREFIX_FINDER_H_
