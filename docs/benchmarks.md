# Benchmarks

The `paper/benchmarks/` directory contains scripts for the paper experiments and
plots.

## Quick Run

```bash
python ./paper/benchmarks/run_benchmarks.py --quick
```

## Notes

- Metaheuristics are stochastic; use multiple runs for stable comparisons.
- CP/MIP backends may require external dependencies (CPLEX, OR-Tools, HiGHS).
- Results are saved under `paper/benchmarks/results`.
