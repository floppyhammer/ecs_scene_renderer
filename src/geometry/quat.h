#ifndef ESR_GEOMETRY_QUAT_H
#define ESR_GEOMETRY_QUAT_H

#include <Eigen/Geometry>

#include "mat3.h"

namespace esr {

class Quat {
public:
    Quat() {
        quaternion.setIdentity();
    }

    Quat(Eigen::Quaternionf eigen_quat) {
        quaternion = eigen_quat;
    }

    Mat3 to_matrix() const {
        auto eigen_mat = quaternion.matrix();
        Mat3 result;
        std::memcpy(result.v.data(), eigen_mat.data(), sizeof(Mat3));
        return result;
    }

    static Quat from_matrix(const Mat3& matrix) {
        Eigen::Matrix3f eigen_mat;
        std::memcpy(eigen_mat.data(), matrix.v.data(), sizeof(Mat3));
        return {Eigen::Quaternionf(eigen_mat)};
    }

    static Quat from_euler(float roll_radians, float pitch_radians, float yaw_radians) {
        Eigen::Quaternionf q;
        q = Eigen::AngleAxisf(roll_radians, Eigen::Vector3f::UnitX()) *
            Eigen::AngleAxisf(pitch_radians, Eigen::Vector3f::UnitY()) *
            Eigen::AngleAxisf(yaw_radians, Eigen::Vector3f::UnitZ());
        return {q};
    }

    /// Euler from quaternion in roll, pitch, yaw.
    Vec3F to_euler() {
        auto euler = quaternion.toRotationMatrix().eulerAngles(0, 1, 2);
        return {euler.x(), euler.y(), euler.z()};
    }

    static Quat from_xyzw(float x, float y, float z, float w) {
        return {Eigen::Quaternionf(w, x, y, z)};
    }

    /// Creates a quaternion from the `angle` (in radians) around the x axis.
    static Quat from_rotation_x(float angle) {
        return from_xyzw(std::sin(angle * 0.5), 0.0, 0.0, std::cos(angle * 0.5));
    }

    /// Creates a quaternion from the `angle` (in radians) around the y axis.
    static Quat from_rotation_y(float angle) {
        return from_xyzw(0.0, std::sin(angle * 0.5), 0.0, std::cos(angle * 0.5));
    }

    /// Creates a quaternion from the `angle` (in radians) around the z axis.
    static Quat from_rotation_z(float angle) {
        return from_xyzw(0.0, 0.0, std::sin(angle * 0.5), std::cos(angle * 0.5));
    }

private:
    Eigen::Quaternionf quaternion;
};

} // namespace esr

#endif // ESR_GEOMETRY_QUAT_H
