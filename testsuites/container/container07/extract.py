#!/usr/bin/env python3
"""Extract/validate container07 serial output; never compiles or runs RTEMS."""
import argparse
import csv
import json
import math
from pathlib import Path
import statistics

METRICS = {
    'run_ready': 'run', 'start_ready': 'start',
    'run_api_return': 'run', 'start_api_return': 'start',
    'timer_wakeup': 'timer', 'switch_a_to_b': 'switch',
    'switch_b_to_a': 'switch', 'switch_rtt': 'switch',
    'switch_half_rtt': 'switch',
}
FIELDS = ['metric', 'round', 'n', 'min_ns', 'mean_ns', 'p50_ns', 'p95_ns',
          'p99_ns', 'max_ns', 'stddev_ns']


def require(ok, message):
    if not ok:
        raise ValueError(message)


def value(metric, times):
    a, b, c, d = times
    if metric in ('run_ready', 'start_ready', 'timer_wakeup', 'switch_a_to_b'):
        return b - a
    if metric in ('run_api_return', 'start_api_return'):
        return c - a
    if metric == 'switch_b_to_a':
        return d - c
    if metric == 'switch_half_rtt':
        return (d - a) / 2
    return d - a


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('log', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    meta, summaries, raw, timer_info = {}, {}, {}, {}
    done = False
    with args.log.open(encoding='utf-8', errors='replace') as stream:
        for line_number, line in enumerate(stream, 1):
            row = next(csv.reader([line.strip()]))
            if not row or not row[0].startswith('C07'):
                continue
            tag = row[0]
            require(not done, f'Extra records after C07DONE at line {line_number}; use one run per log')
            if tag == 'C07META':
                require(len(row) == 3 and row[1] not in meta, f'Invalid/duplicate metadata: {row}')
                meta[row[1]] = row[2]
            elif tag == 'C07SUMMARY':
                require(len(row) == len(FIELDS) + 1, f'Invalid summary: {row}')
                item = dict(zip(FIELDS, row[1:]))
                require(item['metric'] in METRICS, f'Unknown metric: {item}')
                item['round'], item['n'] = int(item['round']), int(item['n'])
                for field in FIELDS[3:]:
                    item[field] = float(item[field])
                    require(math.isfinite(item[field]) and item[field] >= 0, f'Invalid summary value: {item}')
                key = (item['metric'], item['round'])
                require(key not in summaries, f'Duplicate summary: {key}')
                summaries[key] = item
            elif tag == 'C07RAW':
                require(len(row) == 8 and row[1] in METRICS.values(), f'Invalid raw row: {row}')
                kind, round_no, index = row[1], int(row[2]), int(row[3])
                times = tuple(int(x) for x in row[4:])
                require(min(times) >= 0, f'Negative timestamp: {row}')
                if kind == 'switch':
                    require(list(times) == sorted(times), f'Unordered handoff: {row}')
                else:
                    require(times[1] >= times[0], f'Negative latency: {row}')
                    if kind in ('run', 'start'):
                        require(times[2] >= times[0] and times[3] == 0, f'Invalid startup: {row}')
                    else:
                        require(times[2:] == (0, 0), f'Invalid timer: {row}')
                values = raw.setdefault((kind, round_no), [])
                require(index == len(values), f'Missing/duplicate raw sample: {row[:4]}')
                values.append(times)
            elif tag == 'C07TIMER':
                require(len(row) == 4, f'Invalid timer status: {row}')
                r, overruns, max_missed = map(int, row[1:])
                require(r not in timer_info and min(overruns, max_missed) >= 0, f'Invalid timer status: {row}')
                timer_info[r] = (overruns, max_missed)
            elif tag == 'C07DONE':
                done = True
            else:
                require(tag in ('C07SUMMARY_HEADER', 'C07RAW_HEADER'), f'Unknown record: {tag}')
    require(done, 'Missing C07DONE: measurement or serial capture is incomplete')
    rounds = int(meta['rounds'])
    require(rounds > 0 and meta['raw'] in ('0', '1'), 'Invalid round/raw metadata')
    counts = {kind: int(meta['startup_samples' if kind in ('run', 'start') else kind + '_samples'])
              for kind in ('run', 'start', 'timer', 'switch')}
    require(min(counts.values()) > 0, 'Invalid sample count')
    expected = {(metric, r) for metric in METRICS for r in range(rounds + 1)}
    require(set(summaries) == expected, 'Missing/extra per-round or pooled summaries')
    require(set(timer_info) == set(range(1, rounds + 1)), 'Missing/extra timer status')
    for (metric, r), item in summaries.items():
        require(item['n'] == counts[METRICS[metric]] * (rounds if r == 0 else 1), f'Wrong sample count: {item}')
        require(item['min_ns'] <= item['p50_ns'] <= item['p95_ns'] <= item['p99_ns'] <= item['max_ns'],
                f'Invalid percentile order: {item}')
    if meta['raw'] == '1':
        require(set(raw) == {(k, r) for k in counts for r in range(1, rounds + 1)}, 'Missing/extra raw streams')
        for (kind, r), values in raw.items():
            require(len(values) == counts[kind], f'Wrong raw count: {kind}, round {r}')
            if kind == 'timer':
                period = int(meta['period_us']) * 1000
                require(period > 0, 'Invalid timer period')
                require(all(b[0] - a[0] == period for a, b in zip(values, values[1:])), 'Timer deadline drift')
                missed = [(b - a) // period for a, b, _, _ in values]
                require(timer_info[r] == (sum(x > 0 for x in missed), max(missed)), 'Timer overrun mismatch')
        for (metric, r), item in summaries.items():
            runs = range(1, rounds + 1) if r == 0 else [r]
            values = sorted(value(metric, t) for run in runs for t in raw[(METRICS[metric], run)])
            n = len(values)
            expected_stats = {'min_ns': values[0], 'max_ns': values[-1],
                              'mean_ns': statistics.fmean(values),
                              'stddev_ns': statistics.stdev(values) if n > 1 else 0}
            for percent in (50, 95, 99):
                expected_stats[f'p{percent}_ns'] = values[math.ceil(n * percent / 100) - 1]
            for field, number in expected_stats.items():
                require(math.isclose(item[field], number, rel_tol=1e-10, abs_tol=.002),
                        f'Summary/raw mismatch: {metric}, round {r}, {field}')
    else:
        require(not raw, 'Raw rows contradict raw=0')

    args.output.mkdir(parents=True, exist_ok=False)
    with (args.output / 'summary.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=FIELDS)
        writer.writeheader()
        writer.writerows(summaries.values())
    for kind in counts:
        if meta['raw'] != '1':
            break
        with (args.output / f'{kind}.csv').open('w', newline='') as stream:
            writer = csv.writer(stream)
            writer.writerow(['round', 'sample', 't0_ns', 't1_ns', 't2_ns', 't3_ns'])
            for r in range(1, rounds + 1):
                for i, times in enumerate(raw[(kind, r)]):
                    writer.writerow([r, i, *times])
    manifest = {'status': 'complete', 'metadata': meta, 'timer_status': timer_info,
                'raw_validated': meta['raw'] == '1', 'source_log': str(args.log.resolve())}
    (args.output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(f'Validated {rounds} rounds; results: {args.output}')


if __name__ == '__main__':
    main()
