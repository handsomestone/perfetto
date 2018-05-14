#!/usr/bin/env python
# Copyright (C) 2017 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

""" Script to list all files in a directory filtering by pattern.

Do NOT use this script to pull in sources for GN targets. Globbing inputs is
a bad idea, as it plays very badly with git leaving untracked files around. This
script should be used only for cases where false positives won't affect the
output of the build but just cause spurious re-runs (e.g. as input section of
an "action" target).
"""

import argparse
import fnmatch
import os
import sys

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--filter', default=[], action='append')
  parser.add_argument('--exclude', default=[], action='append')
  parser.add_argument('--root', required=True)
  args = parser.parse_args()

  if not os.path.exists(args.root):
    return 0
  for pardir, dirs, files in os.walk(args.root, topdown=True):
    assert(pardir.startswith(args.root))
    relpar = pardir[len(args.root):]
    dirs[:] = [d for d in dirs if os.path.join(relpar, d) not in args.exclude]
    for fname in files:
      fpath = os.path.join(relpar, fname)
      match = len(args.filter) == 0
      for filter in args.filter:
        if fnmatch.fnmatch(fpath, filter):
          match = True
          break
      if match:
        print fpath

if __name__ == '__main__':
  sys.exit(main())
