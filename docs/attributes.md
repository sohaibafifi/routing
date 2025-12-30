# Attributes

Attributes define the problem features and enable matching constraint
generators and evaluators.

## Core Attributes

- **GeoNode**: Coordinates (distance calculation)
- **Consumer**: Demand at a client
- **Stock**: Vehicle capacity
- **Rendezvous**: Time window at a node
- **ServiceQuery**: Service time at a node

## Additional Attributes

- **Profit**: Optional objective terms for TOP-like problems
- **Pickup/Delivery**: Paired demand constraints
- **Sync**: Temporal synchronization constraints
- **Soft Time Windows**: Penalty-based time windows (CVRPSTW)

## Usage Example (C++)

```cpp
problem.enableAttributes<GeoNode, Consumer, Stock, Rendezvous, ServiceQuery>();
client->addAttribute<GeoNode>(10.0, 20.0);
client->addAttribute<Consumer>(15);
client->addAttribute<Rendezvous>(0.0, 200.0);
client->addAttribute<ServiceQuery>(5.0);
```
