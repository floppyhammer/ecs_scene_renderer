#include "projection.h"

namespace esr {

Mat4 PerspectiveProjection::get_projection_matrix() {
    return Mat4::perspective_infinite_reverse_rh(fov_, aspect_ratio_, near_, far_);
}

void PerspectiveProjection::update(float width, float height) {
    aspect_ratio_ = width / height;
}

float PerspectiveProjection::far() const {
    return far_;
}

Mat4 OrthographicProjection::get_projection_matrix() {
    return Mat4::orthographic_rh(
        area_.min_x(),
        area_.max_x(),
        area_.min_y(),
        area_.max_y(),
        // NOTE: near and far are swapped to invert the depth range from [0,1] to [1,0]
        // This is for interoperability with pipelines using infinite reverse perspective projections.
        far_,
        near_);
}

void OrthographicProjection::update(float width, float height) {
    float projection_width = width;
    float projection_height = height;

    float origin_x = projection_width * viewport_origin_.x;
    float origin_y = projection_height * viewport_origin_.y;

    area_ = {scale_ * -origin_x,
             scale_ * -origin_y,
             scale_ * (projection_width - origin_x),
             scale_ * (projection_height - origin_y)};
}

float OrthographicProjection::far() const {
    return far_;
}

Mat4 C_Projection::get_projection_matrix() {
    if (type_ == Type::Perspective) {
        return perspective_.value().get_projection_matrix();
    }
    return orthographic_.value().get_projection_matrix();
}

void C_Projection::update(float width, float height) {
    if (type_ == Type::Perspective) {
        return perspective_.value().update(width, height);
    }
    return orthographic_.value().update(width, height);
}

float C_Projection::far() const {
    if (type_ == Type::Perspective) {
        return perspective_.value().far();
    }
    return orthographic_.value().far();
}

} // namespace esr
