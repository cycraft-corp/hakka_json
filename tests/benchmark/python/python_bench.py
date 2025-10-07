#!/usr/bin/env python3
import json
import sys
import os

def get_rss_kb():
    """Get RSS (Resident Set Size) in kilobytes from /proc/self/status"""
    try:
        with open('/proc/self/status', 'r') as f:
            for line in f:
                if line.startswith('VmRSS:'):
                    # Extract the number from "VmRSS:    12345 kB"
                    parts = line.split()
                    return int(parts[1])
    except (FileNotFoundError, IndexError, ValueError):
        return -1
    return -1

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <input_file>", file=sys.stderr)
        sys.exit(1)

    # Measure RSS before loading
    rss_before = get_rss_kb()

    json_array = []

    try:
        with open(sys.argv[1], 'r') as input_file:
            for line in input_file:
                line = line.strip()
                if not line:
                    continue
                try:
                    obj = json.loads(line)
                    json_array.append(obj)
                except json.JSONDecodeError as e:
                    print(f"Failed to parse JSON: {e}", file=sys.stderr)
                    continue
    except IOError as e:
        print(f"Failed to open the file: {e}", file=sys.stderr)
        sys.exit(1)

    # Measure RSS after loading
    rss_after = get_rss_kb()

    if rss_before >= 0 and rss_after >= 0:
        rss_diff = rss_after - rss_before
        print(f"python{sys.version_info.major}.{sys.version_info.minor} RSS: {rss_diff} KB")
    else:
        print("RSS measurement not available on this platform", file=sys.stderr)

if __name__ == '__main__':
    main()
