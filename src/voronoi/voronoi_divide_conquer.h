
#pragma once
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
   * @brief Recursive divide and conquer function with depth tracking
   * @param sites Sites to process (sorted by x-coordinate)
   * @param bounds Bounding box
   * @param depth Recursion depth for debugging
   * @param face_to_site Output mapping from face index to site index
   * @return DCEL structure
   */
  DCEL* DivideAndConquerWithDepth(
      const std::vector<Point2D>& sites,
      const VoronoiBounds& bounds,
      int depth,
      std::vector<size_t>& face_to_site) const;
  
  /**
   * @brief Merge two Voronoi diagrams
   * @param left_dcel Left Voronoi diagram
   * @param right_dcel Right Voronoi diagram
   * @param left_sites Sites in left diagram
   * @param right_sites Sites in right diagram
   * @param bounds Bounding box
   * @param left_face_to_site Face-to-site mapping for left diagram
   * @param right_face_to_site Face-to-site mapping for right diagram
   * @param out_face_to_site Output face-to-site mapping for merged diagram
   * @return Merged DCEL structure
   */
  DCEL* MergeDiagrams(
      DCEL* left_dcel,
      DCEL* right_dcel,
      const std::vector<Point2D>& left_sites,
      const std::vector<Point2D>& right_sites,
      const VoronoiBounds& bounds,
      const std::vector<size_t>& left_face_to_site,
      const std::vector<size_t>& right_face_to_site,
      std::vector<size_t>& out_face_to_site) const;
  
  /**
   * @brief Clip faces by the dividing curve using face-to-site mapping
   * @param dcel DCEL structure to modify
   * @param left_sites Sites in left diagram
   * @param right_sites Sites in right diagram
   * @param all_sites Combined sites (left + right)
   * @param face_to_site Mapping from face index to site index in all_sites
   */
  void ClipFacesByDividingCurve(
      DCEL* dcel,
      const std::vector<Point2D>& left_sites,
      const std::vector<Point2D>& right_sites,
      const std::vector<Point2D>& all_sites,
      const std::vector<size_t>& face_to_site) const;

  // Face-to-site mapping (stored during DCEL generation)
  mutable std::vector<size_t> face_to_site_mapping_;

  // Sorted sites (stored during DCEL generation for correct index mapping)
  mutable std::vector<Point2D> sorted_sites_;
};

}  // namespace geometry

