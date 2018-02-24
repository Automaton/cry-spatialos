#pragma once
#include <improbable/worker.h>

/**
 * The View is an optional data structure that maintains the known state of entities in the worker's
 * view of the simulation. This object should not be modified concurrently by multiple threads. In
 * particular, a call to Process() can modify the Entities map as a side-effect, and so should not
 * be made concurrently with any manual modification of the Entities map.
 *
 * Note that as of SpatialOS 12 this class is intended primarily as an example: using the
 * worker::ForEachComponent functionality, a custom View can be implemented from scratch with any
 * semantics desired.
 */


class CSpatialOsView : public worker::Dispatcher {
public:
  template <typename... T>
  CSpatialOsView(const worker::Components<T...>& components) : worker::Dispatcher{components} {
    OnAddEntity([this](const worker::AddEntityOp& op) {
      Entities[op.EntityId];
      ComponentAuthority[op.EntityId];
    });
    OnRemoveEntity([this](const worker::RemoveEntityOp& op) {
      Entities.erase(op.EntityId);
      ComponentAuthority.erase(op.EntityId);
    });
    worker::ForEachComponent(components, TrackComponentHandler{*this});
  }

  // Not copyable or movable.
  CSpatialOsView(const CSpatialOsView&) = delete;
  CSpatialOsView(CSpatialOsView&&) = delete;
  CSpatialOsView& operator=(const CSpatialOsView&) = delete;
  CSpatialOsView& operator=(CSpatialOsView&&) = delete;

  /**
   * Helper function that checks if the worker has authority over a particular component of a
   * particular entity. The template parameter T should be a generated component metaclass.
   */
  template <typename T>
  worker::Authority GetAuthority(worker::EntityId entity_id) const {
    auto it = ComponentAuthority.find(entity_id);
    if (it == ComponentAuthority.end()) {
      return worker::Authority::kNotAuthoritative;
    }
    auto jt = it->second.find(T::ComponentId);
    return jt == it->second.end() ? worker::Authority::kNotAuthoritative : jt->second;
  }

  /** Current component data for all entities in the worker's view. */
  worker::Map<worker::EntityId, worker::Entity> Entities;
  /** Current authority delegations. */
  worker::Map<worker::EntityId, worker::Map<worker::ComponentId, worker::Authority>> ComponentAuthority;

private:
  struct TrackComponentHandler {
    CSpatialOsView& view_reference;

    template <typename T>
    void Accept() const {
      CSpatialOsView& view = this->view_reference;

      view.OnAddComponent<T>([&view](const worker::AddComponentOp<T>& op) {
        auto it = view.Entities.find(op.EntityId);
        if (it != view.Entities.end()) {
          worker::Entity& entity = it->second;
          entity.Add<T>(op.Data);
          view.ComponentAuthority[op.EntityId][T::ComponentId] = worker::Authority::kNotAuthoritative;
        }
      });

      view.OnRemoveComponent<T>([&view](const worker::RemoveComponentOp& op) {
        auto it = view.Entities.find(op.EntityId);
        if (it != view.Entities.end()) {
          worker::Entity& entity = it->second;
          entity.Remove<T>();
        }

        auto jt = view.ComponentAuthority.find(op.EntityId);
        if (jt != view.ComponentAuthority.end()) {
          jt->second.erase(T::ComponentId);
        }
      });

      view.OnAuthorityChange<T>([&view](const worker::AuthorityChangeOp& op) {
        view.ComponentAuthority[op.EntityId][T::ComponentId] = op.Authority;
      });

      view.OnComponentUpdate<T>([&view](const worker::ComponentUpdateOp<T>& op) {
        auto it = view.Entities.find(op.EntityId);
        if (it != view.Entities.end()) {
          worker::Entity& entity = it->second;
          if (entity.Get<T>()) {
            entity.Update<T>(op.Update);
          }
        }
      });
    }
  };
};

