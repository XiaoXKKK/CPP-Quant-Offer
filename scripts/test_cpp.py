"""Compile canonical demos and runnable/compile-only Markdown blocks on Linux."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('--compiler', default=os.environ.get('CXX', 'g++'))
parser.add_argument('--sanitize', choices=['none', 'address', 'thread'], default='none')
args = parser.parse_args()
root = Path(__file__).resolve().parent.parent
out = root / '.cpp-build'
out.mkdir(exist_ok=True)
cases = [(file, True) for file in sorted((root / 'examples').glob('*.cpp'))]
for doc in sorted((root / 'content' / 'topics').rglob('*')):
    if doc.suffix not in ['.md', '.mdx']:
        continue
    for index, (kind, source) in enumerate(re.findall(r'```cpp (runnable|compile-only)\n(.*?)\n```', doc.read_text(encoding='utf-8'), re.S)):
        target = out / f'{doc.stem}-block-{index}.cpp'
        target.write_text(source, encoding='utf-8')
        cases.append((target, kind == 'runnable'))
flags = ['-std=c++20', '-O2', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-pthread']
if args.sanitize != 'none':
    flags = [flag for flag in flags if flag != '-O2'] + ['-O1', '-g', '-fno-omit-frame-pointer', f'-fsanitize={"address,undefined" if args.sanitize == "address" else "thread"}']
    # Non-PIE avoids ASan shadow mapping collisions on some WSL kernels.
    if args.sanitize == 'address':
        flags += ['-fno-pie', '-no-pie']
report = []
for source, runnable in cases:
    binary = out / (source.stem + ('-san' if args.sanitize != 'none' else ''))
    command = [args.compiler, *flags, str(source), '-o', str(binary)]
    if not runnable:
        command += ['-c']
    subprocess.run(command, check=True, timeout=90)
    if runnable:
        result = subprocess.run([str(binary)], timeout=30, text=True, capture_output=True)
        if result.returncode != 0:
            print(result.stdout)
            print(result.stderr)
            raise SystemExit(f'FAIL {source.name}: exit {result.returncode}')
        print(f'PASS {source.name}\n{result.stdout.strip()}')
        report.append({'file': source.name, 'stdout': result.stdout.strip()})
    else:
        print(f'PASS compile-only {source.name}')
print(f'{len(cases)} C++ cases passed ({args.compiler}; sanitizer={args.sanitize}).')
(out / 'results.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
