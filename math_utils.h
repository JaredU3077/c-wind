// math_utils.h - Utility functions for common mathematical operations in Browserwind
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "raylib.h"
#include <cmath>

/**
 * Browserwind Math Utilities
 * Provides optimized and reusable mathematical functions for 3D game operations
 */

namespace MathUtils {

// ============================================================================
// VECTOR OPERATIONS
// ============================================================================

/**
 * Calculate the Euclidean distance between two 3D points
 * @param a First point
 * @param b Second point
 * @return Distance between points
 */
inline float distance3D(const Vector3& a, const Vector3& b) {
    Vector3 diff = {a.x - b.x, a.y - b.y, a.z - b.z};
    return sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
}

/**
 * Calculate the squared distance between two 3D points (faster, no sqrt)
 * Useful for distance comparisons where actual distance isn't needed
 * @param a First point
 * @param b Second point
 * @return Squared distance between points
 */
inline float distanceSquared3D(const Vector3& a, const Vector3& b) {
    Vector3 diff = {a.x - b.x, a.y - b.y, a.z - b.z};
    return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
}

/**
 * Calculate the length (magnitude) of a 3D vector
 * @param v Vector to measure
 * @return Length of the vector
 */
inline float vectorLength3D(const Vector3& v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * Calculate the squared length of a 3D vector (faster, no sqrt)
 * @param v Vector to measure
 * @return Squared length of the vector
 */
inline float vectorLengthSquared3D(const Vector3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

/**
 * Normalize a 3D vector (make it unit length)
 * @param v Vector to normalize
 * @return Normalized vector (or zero vector if input was zero)
 */
inline Vector3 normalizeVector3D(const Vector3& v) {
    float length = vectorLength3D(v);
    if (length > 0.0f) {
        return Vector3{v.x / length, v.y / length, v.z / length};
    }
    return Vector3{0.0f, 0.0f, 0.0f}; // Return zero vector if input was zero
}

/**
 * Check if a vector is approximately zero length
 * @param v Vector to check
 * @param epsilon Tolerance for "zero" (default: 1e-6f)
 * @return true if vector is approximately zero
 */
inline bool isVectorZero3D(const Vector3& v, float epsilon = 1e-6f) {
    return vectorLengthSquared3D(v) < epsilon * epsilon;
}

// ============================================================================
// 2D VECTOR OPERATIONS
// ============================================================================

/**
 * Calculate the Euclidean distance between two 2D points
 * @param a First point
 * @param b Second point
 * @return Distance between points
 */
inline float distance2D(const Vector2& a, const Vector2& b) {
    Vector2 diff = {a.x - b.x, a.y - b.y};
    return sqrtf(diff.x * diff.x + diff.y * diff.y);
}

/**
 * Calculate the squared distance between two 2D points (faster, no sqrt)
 * @param a First point
 * @param b Second point
 * @return Squared distance between points
 */
inline float distanceSquared2D(const Vector2& a, const Vector2& b) {
    Vector2 diff = {a.x - b.x, a.y - b.y};
    return diff.x * diff.x + diff.y * diff.y;
}

/**
 * Calculate the length (magnitude) of a 2D vector
 * @param v Vector to measure
 * @return Length of the vector
 */
inline float vectorLength2D(const Vector2& v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

/**
 * Normalize a 2D vector (make it unit length)
 * @param v Vector to normalize
 * @return Normalized vector (or zero vector if input was zero)
 */
inline Vector2 normalizeVector2D(const Vector2& v) {
    float length = vectorLength2D(v);
    if (length > 0.0f) {
        return Vector2{v.x / length, v.y / length};
    }
    return Vector2{0.0f, 0.0f}; // Return zero vector if input was zero
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Clamp a value between min and max
 * @param value Value to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped value
 */
template<typename T>
inline T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * Linear interpolation between two values
 * @param a Start value
 * @param b End value
 * @param t Interpolation factor (0.0 = a, 1.0 = b)
 * @return Interpolated value
 */
template<typename T>
inline T lerp(T a, T b, float t) {
    return a + (b - a) * t;
}

/**
 * Check if two floating-point values are approximately equal
 * @param a First value
 * @param b Second value
 * @param epsilon Tolerance for equality
 * @return true if values are approximately equal
 */
inline bool approximatelyEqual(float a, float b, float epsilon = 1e-6f) {
    return fabsf(a - b) < epsilon;
}

/**
 * Convert degrees to radians
 * @param degrees Angle in degrees
 * @return Angle in radians
 */
inline float degreesToRadians(float degrees) {
    return degrees * (PI / 180.0f);
}

/**
 * Convert radians to degrees
 * @param radians Angle in radians
 * @return Angle in degrees
 */
inline float radiansToDegrees(float radians) {
    return radians * (180.0f / PI);
}

} // namespace MathUtils

#endif // MATH_UTILS_H
