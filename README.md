# RandomPolygonGenerator
A simple program to generate random polygons based on 3 fixed criteria:

1. Generate a Star Polygon with the point set centroid as angle center.
2. Monotone polygon with leftmost and rightmost points as defining line.
3. A random polygon based on Delaunay Triangulation which (tries to) create a spanning polygon for the point set.

The point set is randomly generated and number of points can be passed as a command line argument, default 100.

The Triangulation library used is the fast Delaunay Triangulation Library [delaunator-cpp](https://github.com/delfrrr/delaunator-cpp)

Interface:

Key | Action
--- | ---
s | Star Polygon
m | Monotone Polygon
r | Random Polygon
c | Clear Polgon
q | Quit


Star Polygon:
![alt-text](https://github.com/KuZo1304/RandomPolygonGenerator/Star.png "Star Polygon")

Monotone Polygon:
![alt-text](https://github.com/KuZo1304/RandomPolygonGenerator/Mono.png "Monetone Polygon")

Random Polygon:
![alt-text](https://github.com/KuZo1304/RandomPolygonGenerator/Rand.png "Random Polygon")
