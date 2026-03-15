/**
 * @file delaunay_benchmark.cc
 * @brief Performance comparison between naive and optimized Delaunay triangulation
 * 
 * This demo compares the performance of:
 * 1. Naive implementation (without spatial grid)
 * 2. Optimized implementation (with spatial grid)
 * 
 * Usage:
 *   Run the program and observe the time difference
 * 
 * Expected Results:
 * - Small datasets (< 100 points): Similar performance
 * - Medium datasets (100-1000 points): Optimized is 2-5x faster
 * - Large datasets (> 1000 points): Optimized is 10-100x faster
 */

#include "../../src/triangulation/delaunay_triangulation.h"
#include "../../src/triangulation/delaunay_triangulation_optimized.h"
#include "../../src/core/point2d.h"
#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>

using namespace geometry;

/**
 * @brief Generate random points in a unit square
 * @param count Number of points to generate
 * @param seed Random seed
 * @return Vector of random points
 */
std::vector<Point2D> GenerateRandomPoints(size_t count, unsigned int seed = 42) {
  std::vector<Point2D> points;
  points.reserve(count);
  
  std::mt19937 gen(seed);
  std::uniform_real_distribution<double> dis(0.0, 1000.0);
  
  for (size_t i = 0; i < count; ++i) {
    points.emplace_back(dis(gen), dis(gen));
  }
  
  return points;
}

/**
 * @brief Benchmark a triangulation algorithm
 * @param name Algorithm name
 * @param algorithm Triangulation algorithm to test
 * @param points Input points
 * @return Time taken in milliseconds
 */
double BenchmarkAlgorithm(
    const std::string& name,
    ITriangulationAlgorithm* algorithm,
    const std::vector<Point2D>& points) {
  
  std::cout << "\n========================================" << std::endl;
  std::cout << "Running: " << name << std::endl;
  std::cout << "Points: " << points.size() << std::endl;
  std::cout << "========================================" << std::endl;
  
  auto start = std::chrono::high_resolution_clock::now();
  
  TriangulationResult result = algorithm->Triangulate(points);
  
  auto end = std::chrono::high_resolution_clock::now();
  double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
  
  std::cout << "\nResults:" << std::endl;
  std::cout << "  Triangles: " << result.triangles.size() << std::endl;
  std::cout << "  Vertices: " << result.vertices.size() << std::endl;
  std::cout << "  Edges: " << result.edges.size() << std::endl;
  std::cout << "  Time: " << std::fixed << std::setprecision(2) 
            << elapsed_ms << " ms" << std::endl;
  
  return elapsed_ms;
}

/**
 * @brief Run performance comparison
 */
int main() {
  std::cout << "========================================" << std::endl;
  std::cout << "Delaunay Triangulation Benchmark" << std::endl;
  std::cout << "========================================" << std::endl;
  
  // Test different dataset sizes
  std::vector<size_t> dataset_sizes = {
    100,      // Small
    500,      // Medium
    1000,     // Large
    2000,     // Very large
    5000      // Extreme
  };
  
  std::cout << "\nDataset sizes to test: ";
  for (size_t size : dataset_sizes) {
    std::cout << size << " ";
  }
  std::cout << std::endl;
  
  // Summary table
  std::cout << "\n" << std::string(80, '=') << std::endl;
  std::cout << std::left << std::setw(15) << "Points"
            << std::setw(25) << "Naive (ms)"
            << std::setw(25) << "Optimized (ms)"
            << std::setw(15) << "Speedup" << std::endl;
  std::cout << std::string(80, '-') << std::endl;
  
  for (size_t size : dataset_sizes) {
    // Generate random points
    std::vector<Point2D> points = GenerateRandomPoints(size);
    
    // Test naive implementation
    DelaunayTriangulation naive;
    double naive_time = BenchmarkAlgorithm(
        "Naive Delaunay (No Spatial Grid)", &naive, points);
    
    // Test optimized implementation
    DelaunayTriangulationOptimized optimized(true);
    double optimized_time = BenchmarkAlgorithm(
        "Optimized Delaunay (With Spatial Grid)", &optimized, points);
    
    // Calculate speedup
    double speedup = naive_time / optimized_time;
    
    // Print summary
    std::cout << std::string(80, '-') << std::endl;
    std::cout << std::left << std::setw(15) << size
              << std::setw(25) << std::fixed << std::setprecision(2) << naive_time
              << std::setw(25) << optimized_time
              << std::setw(15) << std::fixed << std::setprecision(1) << speedup << "x"
              << std::endl;
  }
  
  std::cout << std::string(80, '=') << std::endl;
  
  // Analysis
  std::cout << "\n" << std::string(80, '=') << std::endl;
  std::cout << "ANALYSIS" << std::endl;
  std::cout << std::string(80, '=') << std::endl;
  std::cout << "\nKey Observations:" << std::endl;
  std::cout << "1. Small datasets (< 100 points): Overhead of spatial grid may not be worth it" << std::endl;
  std::cout << "2. Medium datasets (100-1000 points): Spatial grid starts to show benefits" << std::endl;
  std::cout << "3. Large datasets (> 1000 points): Spatial grid provides significant speedup" << std::endl;
  std::cout << "4. The speedup increases with dataset size (better scalability)" << std::endl;
  
  std::cout << "\nWhen to use Spatial Grid:" << std::endl;
  std::cout << "✓ Large point sets (> 1000 points)" << std::endl;
  std::cout << "✓ Need for fast performance" << std::endl;
  std::cout << "✓ Memory is not a constraint" << std::endl;
  
  std::cout << "\nWhen to use Naive Implementation:" << std::endl;
  std::cout << "✓ Small point sets (< 500 points)" << std::endl;
  std::cout << "✓ Memory is constrained" << std::endl;
  std::cout << "✓ Simplicity is preferred" << std::endl;
  
  std::cout << "\n" << std::string(80, '=') << std::endl;
  
  return 0;
}
