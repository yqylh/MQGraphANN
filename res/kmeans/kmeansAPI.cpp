#include <iostream>
#include <vector>
#include "kmeans.cpp"


void kmeans() {

  int num_clusters = 10;
  std::vector<Point> points;
  Point p(numbers);
  points->push_back(p);
  KMeans kmeans(num_clusters);
  kmeans.init(points);
  kmeans.run();
  kmeans.printMeans();
}
