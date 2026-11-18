#ifndef ESR_TRANSFORM_COMPONENTS_H
#define ESR_TRANSFORM_COMPONENTS_H

#include <optional>

#include "../geometry/mat3.h"
#include "../geometry/mat4.h"
#include "../geometry/quat.h"
#include "../geometry/vec3.h"

namespace esr {

struct C_Transform {
    Vec3F translation;
    Quat rotation;
    Vec3F scale = Vec3F(1.0);

    static C_Transform from_matrix(const Mat4& matrix) {
        C_Transform transform;
        transform.translation = {matrix.v[12], matrix.v[13], matrix.v[14]};
        transform.scale = {sqrtf(pow(matrix.v[0], 2.0f) + pow(matrix.v[1], 2.0f) + pow(matrix.v[2], 2.0f)),
                           sqrtf(pow(matrix.v[4], 2.0f) + pow(matrix.v[5], 2.0f) + pow(matrix.v[6], 2.0f)),
                           sqrtf(pow(matrix.v[8], 2.0f) + pow(matrix.v[9], 2.0f) + pow(matrix.v[10], 2.0f))};

        Mat3 rotation_matrix = {{
            matrix.v[0] / transform.scale.x,
            matrix.v[1] / transform.scale.x,
            matrix.v[2] / transform.scale.x,
            matrix.v[4] / transform.scale.y,
            matrix.v[5] / transform.scale.y,
            matrix.v[6] / transform.scale.y,
            matrix.v[8] / transform.scale.z,
            matrix.v[9] / transform.scale.z,
            matrix.v[10] / transform.scale.z,
        }};
        transform.rotation = Quat::from_matrix(rotation_matrix);

        return transform;
    }

    /// Rotates this `Transform` so that `Transform::forward` points towards the `target` position,
    /// and `Transform::up` points towards `up`.
    ///
    /// In some cases it's not possible to construct a rotation. Another axis will be picked in those cases:
    /// * if `target` is the same as the transform translation, `Vec3::Z` is used instead.
    /// * if `up` is zero, `Vec3::Y` is used instead.
    /// * if the resulting forward direction is parallel with `up`, an orthogonal vector is used as the "right"
    /// direction.
    void look_at(Vec3F target, Vec3F up) {
        look_to(target - translation, up);
    }

    /// Rotates this `Transform` so that `Transform::forward` points in the given `direction`
    /// and `Transform::up` points towards `up`.
    ///
    /// In some cases it's not possible to construct a rotation. Another axis will be picked in those cases:
    /// * if `direction` is zero, `Vec3::Z` is used instead.
    /// * if `up` is zero, `Vec3::Y` is used instead.
    /// * if `direction` is parallel with `up`, an orthogonal vector is used as the "right" direction.
    void look_to(Vec3F direction, Vec3F up) {
        auto forward = -(direction.try_normalize().value_or(Vec3F::Z()));

        up = up.try_normalize().value_or(Vec3F::Y());

        auto right = up.cross(forward).try_normalize().value_or(up.any_orthonormal_vector());
        up = forward.cross(right);
        rotation = Quat::from_matrix(Mat3::from_cols(right, up, forward));
    }
};

struct C_GlobalTransform {
    Mat3 matrix;
    Vec3F translation;

    C_GlobalTransform() {
        matrix = Mat3(1.0);
    }

    C_GlobalTransform(const Mat3& matrix, const Vec3F& translation) : matrix(matrix), translation(translation) {
    }

    C_GlobalTransform(const C_Transform& transform) {
        matrix = transform.rotation.to_matrix() * Mat3::from_scale(transform.scale);
        translation = transform.translation;
    }

    Mat4 compute_matrix() const {
        Mat4 result;
        result.v[0] = matrix.v[0];
        result.v[1] = matrix.v[1];
        result.v[2] = matrix.v[2];

        result.v[4] = matrix.v[3];
        result.v[5] = matrix.v[4];
        result.v[6] = matrix.v[5];

        result.v[8] = matrix.v[6];
        result.v[9] = matrix.v[7];
        result.v[10] = matrix.v[8];

        result.v[12] = translation.x;
        result.v[13] = translation.y;
        result.v[14] = translation.z;
        result.v[15] = 1.0;

        return result;
    }

    Vec3F transform_point(const Vec3F& point) const {
        return matrix * point + translation;
    }

    Vec3F operator*(const Vec3F& v) const {
        return {matrix * v + translation};
    }

    C_GlobalTransform operator*(const C_GlobalTransform& rhs) const {
        return {matrix * rhs.matrix, *this * rhs.translation};
    }
};

} // namespace esr

#endif // ESR_TRANSFORM_COMPONENTS_H
