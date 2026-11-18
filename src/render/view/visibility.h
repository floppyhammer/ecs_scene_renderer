#ifndef ESR_RENDER_VIEW_VISIBILITY_H
#define ESR_RENDER_VIEW_VISIBILITY_H

namespace esr {

/// Collection of entities visible from the current view.
///
/// This component contains all entities which are visible from the currently
/// rendered view. The collection is updated automatically by the `check_visibility()`
/// system, and renderers can use it to optimize rendering of a particular view, to
/// prevent drawing items not visible from that view.
///
/// This component is intended to be attached to the same entity as the `Camera` and
/// the `Frustum` defining the view.
///
/// Currently this component is ignored by the sprite renderer, so sprite rendering
/// is not optimized per view.
struct C_VisibleEntities {
    std::vector<Entity> entities;
};

} // namespace esr

#endif // ESR_RENDER_VIEW_VISIBILITY_H
