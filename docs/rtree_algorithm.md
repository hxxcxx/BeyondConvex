# R-Tree Algorithm

## Overview

R-tree is a tree data structure used for spatial indexing of rectangle objects. Unlike KD-tree and Quadtree which are optimized for point data, R-tree is specifically designed for handling spatial objects with extent, such as:

- Rectangles and bounding boxes
- Polygons
- 2D/3D geometric objects
- GIS data (buildings, roads, etc.)

## Key Characteristics

### Advantages over Point-based Trees

1. **Handles Spatial Extent**: Stores actual rectangle objects, not just points
2. **Efficient Range Queries**: Optimized for finding objects that intersect with a query region
3. **Dynamic**: Supports efficient insertion and deletion
4. **Balanced**: Maintains balance through node splitting algorithms
5. **Disk-friendly**: Designed for secondary storage (original use case)

### Structure

- **Leaf Nodes**: Contain actual rectangle data with associated IDs
- **Internal Nodes**: Contain minimum bounding rectangles (MBRs) that enclose all entries in child nodes
- **Node Capacity**: Each node can hold between `min_entries` and `max_entries` entries

## Algorithm Details

### Insertion

1. **Choose Subtree**: Starting from root, recursively choose the child node whose MBR requires least area enlargement to include the new rectangle
2. **Leaf Node Insertion**: When reaching a leaf, add the new rectangle
3. **Node Splitting**: If a node overflows (exceeds `max_entries`), split it using quadratic split algorithm
4. **Tree Adjustment**: Propagate changes up the tree, updating MBRs and splitting nodes as needed

### Quadratic Split Algorithm

1. **Pick Seeds**: Choose two entries that would waste the most area if placed in the same group
2. **Distribute Entries**: For each remaining entry, assign it to the group that causes less area enlargement
3. **Create New Node**: Move one group to a new node

### Query Operations

#### Range Query
- Find all rectangles that intersect with a query rectangle
- Traverse tree, only visiting subtrees whose MBR intersects with query range

#### Point Query
- Find all rectangles containing a specific point
- Special case of range query with zero-area query rectangle

#### Intersection Query
- Find all rectangles that intersect with a given rectangle
- Same as range query

## Implementation Details

### Data Structures

```cpp
// Entry in R-tree node
struct RTreeEntry {
  BoundingBox mbr;              // Minimum bounding rectangle
  std::unique_ptr<RTreeNode> child;  // Child node (null for leaf entries)
  int data_id;                  // Data ID (only used in leaf entries)
};

// R-tree node
class RTreeNode {
  bool is_leaf_;                // True if leaf node
  std::vector<RTreeEntry> entries_;
  BoundingBox mbr_;             // MBR of all entries
};
```

### Parameters

- **max_entries**: Maximum entries per node (default: 4)
- **min_entries**: Minimum entries per node (default: 2, should be ≤ max_entries/2)

### Complexity

- **Insertion**: O(log n) average case
- **Range Query**: O(log n + k) where k is number of results
- **Space**: O(n)

## Usage Example

```cpp
// Create R-tree with custom parameters
RTree rtree(4, 2);  // max_entries=4, min_entries=2

// Insert rectangles
BoundingBox rect1(10, 10, 50, 50);
BoundingBox rect2(60, 60, 100, 100);
rtree.Insert(rect1, 0);
rtree.Insert(rect2, 1);

// Range query
BoundingBox query_range(0, 0, 30, 30);
std::vector<int> results = rtree.RangeQuery(query_range);

// Point query
Point2D point(25, 25);
std::vector<int> point_results = rtree.PointQuery(point);
```

## Demo Scene Features

The R-tree demo scene provides:

### Modes

1. **Insert Rectangles**: Click and drag to create new rectangles
2. **Range Query**: Click and drag to define query range
3. **Point Query**: Click to query rectangles at a point
4. **Intersection Query**: Click and drag to create query rectangle

### Test Data Generation

- **Random Rectangles**: Generate random positioned and sized rectangles
- **Grid Layout**: Create evenly spaced grid of rectangles
- **Overlapping Rectangles**: Generate rectangles with intentional overlaps

### Visualization

- **Rectangles**: Color-coded by index
- **Tree Structure**: MBRs shown with depth-based colors
- **Query Results**: Highlighted in green
- **Statistics**: Real-time query performance metrics

## Comparison with Other Spatial Indexes

| Feature | R-Tree | KD-Tree | Quadtree |
|---------|--------|---------|----------|
| Data Type | Rectangles | Points | Points |
| Range Query | Excellent | Good | Good |
| Insertion | O(log n) | O(log n) | O(log n) |
| Dynamic | Yes | No (usually) | Yes |
| Space Overhead | Medium | Low | High |
| Best Use Case | GIS, Game Objects | Nearest Neighbor | Spatial Partitioning |

## Applications

1. **Geographic Information Systems (GIS)**
   - Spatial queries on maps
   - Finding features within a region

2. **Computer Games**
   - Collision detection
   - Spatial culling for rendering

3. **Database Systems**
   - Spatial indexing for queries
   - Multi-dimensional data

4. **CAD/CAM Systems**
   - Object selection
   - Spatial queries on drawings

## References

- Guttman, A. (1984). "R-trees: a dynamic index structure for spatial searching"
- Beckmann, N., et al. (1990). "The R*-tree: an efficient and robust access method for points and rectangles"
