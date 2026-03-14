#ifndef GEOMETRY_VORONOI_DIVIDE_CONQUER_H_
#define GEOMETRY_VORONOI_DIVIDE_CONQUER_H_

#include "voronoi_algorithm.h"
#include "half_plane_clipper.h"
#include "../dcel/dcel.h"
#include <vector>
#include <algorithm>

namespace geometry {

/**
 * @brief Divide and Conquer Voronoi diagram algorithm
 * 
 * This algorithm computes the Voronoi diagram using a divide and conquer approach:
 * 1. Recursively divide the set of sites into two halves
 * 2. Compute Voronoi diagram for each half
 * 3. Merge the two diagrams along the dividing line
 * 
 * Time Complexity: O(n log n)
 * Space Complexity: O(n)
 * 
 * Advantages:
 * - Faster than incremental algorithms for large datasets
 * - Optimal time complexity
 * - Good cache performance
 * 
 * Disadvantages:
 * - More complex implementation
 * - Harder to understand and debug
 * - Requires sorting
 */
class DivideConquerVoronoi : public IVoronoiAlgorithm {
 public:
  /**
   * @brief Generate Voronoi diagram
   * @param sites Input site points
   * @param bounds Bounding box for the diagram
   * @return Voronoi diagram result
   */
  VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      const VoronoiBounds& bounds) override;
  
  /**
   * @brief Generate Voronoi diagram in DCEL format
   * @param sites Input site points
   * @param bounds Bounding box for the diagram
   * @return Pointer to DCEL structure
   */
  DCEL* GenerateDCEL(
      const std::vector<Point2D>& sites,
      const VoronoiBounds& bounds) override;
  
  /**
   * @brief Get algorithm name
   * @return Algorithm name
   */
  std::string Name() const override {
    return "Divide & Conquer";
  }
  
  /**
   * @brief Get time complexity
   * @return Complexity string
   */
  std::string Complexity() const override {
    return "O(n log n)";
  }
  
  /**
   * @brief Get algorithm description
   * @return Description
   */
  std::string Description() const override {
    return "Divide and conquer algorithm with O(n log n) complexity. "
           "Recursively divides the site set and merges the results. "
           "Optimal for large datasets.";
  }
  
 private:
  /**
   * @brief Recursive divide and conquer function
   * @param sites Sites to process (sorted by x-coordinate)
   * @param bounds Bounding box
   * @return DCEL structure
   */
  DCEL* DivideAndConquer(
      const std::vector<Point2D>& sites,
      const VoronoiBounds& bounds) const;
  
  /**
   * @brief Recursive divide and conquer function with depth tracking
   * @param sites Sites to process (sorted by x-coordinate)
   * @param bounds Bounding box
   * @param depth Recursion depth for debugging
   * @return DCEL structure
   */
  DCEL* DivideAndConquerWithDepth(
      const std::vector<Point2D>& sites,
      const VoronoiBounds& bounds,
      int depth) const;
  
  /**
   * @brief Merge two Voronoi diagrams
   * @param left_dcel Left Voronoi diagram
   * @param right_dcel Right Voronoi diagram
   * @param left_sites Sites in left diagram
   * @param right_sites Sites in right diagram
   * @param bounds Bounding box
   * @return Merged DCEL structure
   */
  DCEL* MergeDiagrams(
      DCEL* left_dcel,
      DCEL* right_dcel,
      const std::vector<Point2D>& left_sites,
      const std::vector<Point2D>& right_sites,
      const VoronoiBounds& bounds) const;
  
  /**
   * @brief Clip faces by the dividing curve
   * @param dcel DCEL structure to modify
   * @param left_sites Sites in left diagram
   * @param right_sites Sites in right diagram
   */
  void ClipFacesByDividingCurve(
      DCEL* dcel,
      const std::vector<Point2D>& left_sites,
      const std::vector<Point2D>& right_sites) const;
};

}  // namespace geometry

#endif  // GEOMETRY_VORONOI_DIVIDE_CONQUER_H_
