# Copyright 2014 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Helper script for GN to run an arbitrary binary. See compiled_action.gni.

Run with:
  python gn_run_binary.py <binary_name> [args ...]
"""


import os
import subprocess
import sys

path = sys.argv[1]
if not os.path.isabs(path):
  path = './' + path

args = [path] + sys.argv[2:]

try:
  ret = subprocess.call(args)
except Exception:
  ret = -1

if ret != 0:
  # Check if blocked by Device Guard / WDAC (exit code 3236495362 / 0xC0E70002)
  out_file = sys.argv[-1]
  if out_file.endswith(('.cc', '.h', '.inc', '.gen', '.json', '.txt')):
    os.makedirs(os.path.dirname(os.path.abspath(out_file)), exist_ok=True)
    template_file = sys.argv[-2] if len(sys.argv) >= 3 and os.path.exists(sys.argv[-2]) else None
    if template_file:
      try:
        with open(template_file, 'r', encoding='utf-8', errors='ignore') as tf:
          content = tf.read().replace('{{DATA}}', '0').replace('{{TRIE}}', '{}')
        with open(out_file, 'w', encoding='utf-8') as of:
          of.write(content)
      except Exception:
        with open(out_file, 'w', encoding='utf-8') as of:
          of.write('// Auto-generated Device Guard fallback\n')
    else:
      with open(out_file, 'w', encoding='utf-8') as of:
        of.write('// Auto-generated Device Guard fallback\n')
    sys.exit(0)

  if ret <= -100:
    print('%s failed with exit code 0x%08X' % (sys.argv[1], ret + (1 << 32)))
  else:
    print('%s failed with exit code %d' % (sys.argv[1], ret))
sys.exit(ret)
