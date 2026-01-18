# Concepts

## Composable Attributes

Problems are built by adding attributes to entities. 
```python
from routing.constants import Attribute

client.add_attribute(Attribute.GEONODE, x=10, y=20)
client.add_attribute(Attribute.CONSUMER, demand=5)
client.add_attribute(Attribute.RENDEZVOUS, open=0, close=100)
```

## Automatic Enabling

Attributes are **automatically enabled** when added—constraints are activated automatically:

- `GEONODE` → Distance objective
- `CONSUMER` + `STOCK` → Capacity constraints  
- `RENDEZVOUS` → Time window constraints

## Problem Types

| Problem | Attributes |
|---------|-----------|
| TSP | `GEONODE` |
| CVRP | `GEONODE`, `CONSUMER`, `STOCK` |
| CVRPTW | `GEONODE`, `CONSUMER`, `STOCK`, `RENDEZVOUS`, `SERVICE_QUERY` |
| TOP | `GEONODE`, `PROFITER`, `RENDEZVOUS` |
| PDVRP | `GEONODE`, `PICKUP`, `DELIVERY`, `STOCK` |
