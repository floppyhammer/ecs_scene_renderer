#ifndef ESR_RENDER_CAMERA_PROJECTION_H
#define ESR_RENDER_CAMERA_PROJECTION_H

#include <optional>

#include "../../geometry/basic.h"
#include "../../geometry/mat4.h"
#include "../../geometry/rect.h"

#undef far

namespace esr {

/// Class to control the projection matrix of a camera.
class CameraProjection {
public:
    virtual ~CameraProjection() = default;
    virtual Mat4 get_projection_matrix() = 0;
    virtual void update(float width, float height) = 0;
    virtual float far() const = 0;
};

/// A 3D camera projection in which distant objects appear smaller than close objects.
class PerspectiveProjection : public CameraProjection {
public:
    Mat4 get_projection_matrix() override;

    void update(float width, float height) override;

    float far() const override;

    /// The vertical field of view (FOV) in radians.
    ///
    /// Defaults to a value of π/4 radians or 45 degrees.
    float fov_ = PI / 4.0;

    /// The aspect ratio (width divided by height) of the viewing frustum.
    ///
    /// The `camera_system` automatically
    /// updates this value when the aspect ratio of the associated window changes.
    ///
    /// Defaults to a value of `1.0`.
    float aspect_ratio_ = 1.0;

    /// The distance from the camera in world units of the viewing frustum's near plane.
    ///
    /// Objects closer to the camera than this value will not be visible.
    ///
    /// Defaults to a value of `0.1`.
    float near_ = 0.1;

    /// The distance from the camera in world units of the viewing frustum's far plane.
    ///
    /// Objects farther from the camera than this value will not be visible.
    ///
    /// Defaults to a value of `1000.0`.
    float far_ = 1000.0;
};

/// Project a 3D space onto a 2D surface using parallel lines, i.e., unlike `PerspectiveProjection`,
/// the size of objects remains the same regardless of their distance to the camera.
///
/// The volume contained in the projection is called the *view frustum*. Since the viewport is rectangular
/// and projection lines are parallel, the view frustum takes the shape of a cuboid.
///
/// Note that the scale of the projection and the apparent size of objects are inversely proportional.
/// As the size of the projection increases, the size of objects decreases.
class OrthographicProjection : public CameraProjection {
public:
    Mat4 get_projection_matrix() override;

    void update(float width, float height) override;

    float far() const override;

    /// The distance of the near clipping plane in world units.
    ///
    /// Objects closer than this will not be rendered.
    ///
    /// Defaults to `0.0`
    float near_ = 0.0;
    /// The distance of the far clipping plane in world units.
    ///
    /// Objects further than this will not be rendered.
    ///
    /// Defaults to `1000.0`
    float far_ = 1000.0;
    /// Specifies the origin of the viewport as a normalized position from 0 to 1, where (0, 0) is the bottom left
    /// and (1, 1) is the top right. This determines where the camera's position sits inside the viewport.
    ///
    /// When the projection scales due to viewport resizing, the position of the camera, and thereby `viewport_origin`,
    /// remains at the same relative point.
    ///
    /// Consequently, this is pivot point when scaling. With a bottom left pivot, the projection will expand
    /// upwards and to the right. With a top right pivot, the projection will expand downwards and to the left.
    /// Values in between will caused the projection to scale proportionally on each axis.
    ///
    /// Defaults to `(0.5, 0.5)`, which makes scaling affect opposite sides equally, keeping the center
    /// point of the viewport centered.
    Vec2I viewport_origin_;
    /// How the projection will scale when the viewport is resized.
    ///
    /// Defaults to `ScalingMode::WindowScale(1.0)`
    //    ScalingMode scaling_mode;
    /// Scales the projection in world units.
    ///
    /// As scale increases, the apparent size of objects decreases, and vice versa.
    ///
    /// Defaults to `1.0`
    float scale_ = 1.0;
    /// The area that the projection covers relative to `viewport_origin`.
    ///
    /// Bevy's [`camera_system`](crate::camera::camera_system) automatically
    /// updates this value when the viewport is resized depending on `OrthographicProjection`'s other fields.
    /// In this case, `area` should not be manually modified.
    ///
    /// It may be necessary to set this manually for shadow projections and such.
    RectF area_;
};

class C_Projection : public CameraProjection {
public:
    // For component emplacement.
    C_Projection() = default;

    C_Projection(const PerspectiveProjection& projection) {
        type_ = Type::Perspective;
        perspective_ = std::make_optional(projection);
    }

    C_Projection(const OrthographicProjection& projection) {
        type_ = Type::Orthographic;
        orthographic_ = std::make_optional(projection);
    }

    Mat4 get_projection_matrix() override;

    void update(float width, float height) override;

    float far() const override;

private:
    enum class Type {
        Perspective,
        Orthographic,
    } type_;

    std::optional<PerspectiveProjection> perspective_;
    std::optional<OrthographicProjection> orthographic_;
};

} // namespace esr

#endif // ESR_RENDER_CAMERA_PROJECTION_H
