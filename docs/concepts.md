# Concepts

## Composable Attributes

Problems are built by adding attributes to entities. 
```python
from routing.constants import Attribute

client.add_attribute(Attribute.GEONODE, 10, 20)    # Location
client.add_attribute(Attribute.CONSUMER, 5)        # Demand
client.add_attribute(Attribute.RENDEZVOUS, 0, 100) # Time window
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
