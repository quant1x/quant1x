#!/usr/bin/env python3

import argparse
import math
import re
import statistics
import subprocess
import sys
from pathlib import Path

DEFAULT_TEST = "quant1x.tests.LMAXDisruptorBenchmark"
BENCH_PATTERN = re.compile(
    r"LMAX Disruptor benchmark: producers=(\d+) consumers=(\d+) events=(\d+) elapsedMs=(\d+) throughput=(\d+) ops/s"
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run the Java LMAX Disruptor benchmark multiple times and summarize stability metrics."
    )
    parser.add_argument(
        "--test",
        default=DEFAULT_TEST,
        help=f"Maven test target to run, default: {DEFAULT_TEST}",
    )
    parser.add_argument(
        "--rounds",
        type=int,
        default=8,
        help="Number of benchmark rounds to execute (default: 8)",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=180,
        help="Per-run timeout in seconds (default: 180)",
    )
    return parser.parse_args()


def run_once(repo_root: Path, test_name: str, timeout: int) -> int:
    cmd = [str(repo_root / "mvnw"), "-q", f"-Dtest={test_name}", "test"]
    result = subprocess.run(cmd, cwd=str(repo_root), capture_output=True, text=True, timeout=timeout)
    combined = (result.stdout or "") + (result.stderr or "")
    match = BENCH_PATTERN.search(combined)
    if match is None:
        print(f"FAILED: benchmark output not found for '{test_name}'")
        print(combined)
        raise RuntimeError(f"benchmark output not found for '{test_name}'")
    throughput = int(match.group(5))
    return throughput


def summarize(runs: list[int]) -> None:
    if not runs:
        raise ValueError("no benchmark runs collected")

    mean = statistics.mean(runs)
    std = statistics.pstdev(runs) if len(runs) > 1 else 0.0
    ci = 1.96 * std / math.sqrt(len(runs)) if len(runs) > 1 else 0.0
    cv = (100.0 * std / mean) if mean else 0.0

    print("--- summary ---")
    print(f"count={len(runs)}")
    print(f"mean={mean:.2f} ops/s")
    print(f"min={min(runs)} ops/s")
    print(f"max={max(runs)} ops/s")
    print(f"stddev={std:.2f} ops/s")
    print(f"95% CI={ci:.2f} ops/s")
    print(f"cv={cv:.2f}%")


def print_overview(args: argparse.Namespace) -> None:
    print("=== LMAX Disruptor benchmark overview ===")
    print("Scenario: multi-producer / multi-consumer throughput benchmark")
    print("Implementation: LMAX Disruptor, Java, JUnit-based benchmark")
    print("Target test: quant1x.tests.LMAXDisruptorBenchmark")
    print("Configuration: producers=4, consumers=4, ring_size=1024, events_per_producer=200000")
    print(f"Rounds: {args.rounds}, timeout_per_run: {args.timeout}s")
    print("Metric: throughput in operations/second, reported as ops/s")
    print("Stability stats: mean, min, max, stddev, 95% CI, coefficient of variation")
    print("========================================")


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parent.parent
    mvnw = repo_root / "mvnw"
    if not mvnw.exists():
        print(f"Maven wrapper not found: {mvnw}", file=sys.stderr)
        return 2

    print_overview(args)

    runs: list[int] = []
    for i in range(1, args.rounds + 1):
        try:
            throughput = run_once(repo_root, args.test, args.timeout)
        except subprocess.TimeoutExpired:
            print(f"RUN {i}: TIMED OUT after {args.timeout}s")
            return 1
        except RuntimeError:
            return 1
        runs.append(throughput)
        print(f"run{i}: throughput={throughput} ops/s")

    summarize(runs)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        print("\nbenchmark interrupted by user", file=sys.stderr)
        raise SystemExit(130)
