#!/usr/bin/env python3
"""Compare two block CSV files by name/code fields.
Usage: python scripts/cmp_blocks.py <file1> <file2>
"""

import csv
import sys

def load_name_code(path):
    rows = {}
    with open(path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for r in reader:
            key = (r['name'], r['code'])
            rows[key] = r
    return rows

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <blocks_file1> <blocks_file2>")
        sys.exit(1)

    f1, f2 = sys.argv[1], sys.argv[2]
    a = load_name_code(f1)
    b = load_name_code(f2)

    print(f'{f1}: {len(a)} records')
    print(f'{f2}: {len(b)} records')

    only_a = set(a.keys()) - set(b.keys())
    only_b = set(b.keys()) - set(a.keys())

    if only_a:
        print(f'\n=== Only in first file ({len(only_a)}) ===')
        for name, code in sorted(only_a):
            print(f'  name={name}, code={code}')

    if only_b:
        print(f'\n=== Only in second file ({len(only_b)}) ===')
        for name, code in sorted(only_b):
            print(f'  name={name}, code={code}')

    if not only_a and not only_b:
        print('\nNo difference in name/code pairs.')

if __name__ == '__main__':
    main()
