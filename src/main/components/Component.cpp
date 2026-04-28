#include "main/components/Component.h"

#include "main/GameObject.h"

void Component::OnObjectRemoved(size_t id) {
    if (owner && owner->GetID() == id) {
        isComponentDeleted = true;
    }
}
