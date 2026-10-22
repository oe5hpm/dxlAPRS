#!/usr/bin/env python3
"""Check lorarx's offset shifter against independently mixed synthetic IQ.

Build lorarx first, then run:
    python3 tests/test_lorarx_offset.py /path/to/lorarx
Only Python's standard library is required. No UDP output is enabled.
"""
import array
import math
from pathlib import Path
import subprocess
import sys
import tempfile


def check(binary, rate, offset):
    samples = 131072
    original = array.array('f')
    reference = array.array('f')
    for n in range(samples):
        phase = 2 * math.pi * ((offset + 1000) * n % rate) / rate
        original.extend((math.cos(phase), math.sin(phase)))
        # lorarx's mixer is sin(phi) + j*cos(phi), i.e. j*exp(-j*phi).
        base = 2 * math.pi * (1000 * n % rate) / rate
        reference.extend((-math.sin(base), math.cos(base)))
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        outputs = []
        for name, iq, shift in [('offset', original, offset),
                                ('reference', reference, 0)]:
            source = tmp / (name + '.iq')
            target = tmp / (name + '.filtered')
            source.write_bytes(iq.tobytes())
            subprocess.run([binary, '-i', str(source), '-f', 'f32',
                            '-r', str(rate), '-o', str(shift), '-b', '7',
                            '-s', '12', '-Y', 'f', str(target)],
                           check=True, stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE)
            result = array.array('f')
            result.frombytes(target.read_bytes())
            outputs.append(result)
        actual, expected = outputs
        assert len(actual) == len(expected), 'filtered output length changed'
        # Discard startup transient; compare I and Q, including block boundaries.
        assert len(actual) > 4096, 'insufficient filtered output'
        error = sum((a-b)**2 for a, b in zip(actual[2048:], expected[2048:]))
        energy = sum(v*v for v in expected[2048:])
        relative_rms = math.sqrt(error / energy)
        assert relative_rms < 1e-4, (rate, offset, relative_rms)
        print('PASS rate=%d offset=%d relative RMS error=%.3g' %
              (rate, offset, relative_rms))


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise SystemExit('usage: test_lorarx_offset.py /path/to/lorarx')
    binary = str(Path(sys.argv[1]).resolve())
    for rate, offset in [(960000, 300000), (2400000, 975000),
                         (2400000, 1125000), (2400000, -975000)]:
        check(binary, rate, offset)
