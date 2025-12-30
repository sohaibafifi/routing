# Concepts

## Composable Attributes

Problems are built by enabling attributes (GeoNode, Consumer, Stock, Rendezvous,
ServiceQuery, etc.) and attaching them to entities at runtime. This removes the
need for deep inheritance hierarchies.

## Constraint Registry

Constraint generators auto-register based on enabled attributes. When you call
`problem.enableAttributes<...>()`, the registry activates routing, capacity,
time window, and other generators in priority order.

## Evaluators

Evaluators check feasibility and insertion deltas for metaheuristics. When
attributes are enabled, the corresponding evaluators become active and drive
constraint-aware decoding and local search.

## Legacy Compatibility

The legacy hierarchy (VRP -> CVRP -> CVRPTW) is still supported. The composable
problem is the preferred interface for new work.
