#include "main/components/constrainttypes/Pin.h"

#include <algorithm>

#include "external/imgui/imgui.h"

#include "main/GameObject.h"
#include "main/World.h"
#include "main/components/Constraint.h"
#include "main/editor/EditorState.h"
#include "main/physics/Config.h"
#include "math/Matrix2x2.h"
#include "math/RotationMatrix.h"

PinConstraint::PinConstraint(std::vector<PinAttachment> attachments, Vec2 pos, bool fixedX, bool fixedY)
    : attachments(std::move(attachments)), fixedX(fixedX), fixedY(fixedY)
{ 
    this->position = pos;
}

ConstraintType PinConstraint::GetType() const
{
    return ConstraintType::PIN;
}

void PinConstraint::Solve(float dt)
{
    for (PinAttachment& att : attachments)
    {
        GameObject* obj = att.obj;
        RigidBody* rb = obj->rb;
        if (!rb) continue;

        Vec2 localAnchor(att.localX, att.localY);
        Vec2 r = RotMatrix(obj->transform.rotation).Rotate(localAnchor);

        Vec2 worldAnchor = obj->transform.position + r;
        Vec2 delta = position - worldAnchor;

        Vec2 v = rb->GetVelocity();
        float w = rb->GetAngularVelocity();
        
        Vec2 pointVelocity(v.x - w * r.y, v.y + w * r.x);
        Vec2 biasVelocity = delta * (Config::biasConstraint / dt);
        Vec2 targetVelocity = biasVelocity - pointVelocity;
        
        //k
        float iI = rb->GetInvInertia();
        float iM = rb->GetInvMass();

        if (fixedX && fixedY)
        {
            Matrix2x2 K(
                iM + (r.y * r.y) * iI,
                -r.x * r.y * iI,
                -r.x * r.y * iI,       
                iM + (r.x * r.x) * iI
            );

            Matrix2x2 K_inv = K.Inverse();
            Vec2 impulse = K_inv * targetVelocity;

            rb->SetVelocity(v + impulse * iM);
            rb->SetAngularVelocity(w + (r.Cross(impulse) * iI));
        }
        else if (fixedX)
        {
            float effMassX = iM + (r.y * r.y) * iI;
            if (effMassX > 0.0f)
            {
                float impulseX = targetVelocity.x / effMassX;
                rb->SetVelocity(Vec2(v.x + impulseX * iM, v.y));
                rb->SetAngularVelocity(w + (r.Cross(Vec2(impulseX, 0.0f)) * iI));
            }
        }
        else if (fixedY)
        {
            float effMassY = iM + (r.x * r.x) * iI;
            if (effMassY > 0.0f)
            {
                float impulseY = targetVelocity.y / effMassY;
                rb->SetVelocity(Vec2(v.x, v.y + impulseY * iM));
                rb->SetAngularVelocity(w + (r.Cross(Vec2(0.0f, impulseY)) * iI));
            }
        }

        if (!fixedX) position.x = worldAnchor.x;
        if (!fixedY) position.y = worldAnchor.y;
    }
}

void PinConstraint::OnObjectRemoved(size_t id)
{
    Component::OnObjectRemoved(id);
    attachments.erase(std::remove_if(attachments.begin(), attachments.end(), [id](const PinAttachment& a) {
        return a.obj->GetID() == id;
    }), attachments.end());
}

bool PinConstraint::IsInvalid() const
{
    return isComponentDeleted || attachments.empty();
}

bool PinConstraint::InvolvesObject(GameObject* obj) const
{
    for (const auto& att : attachments) if (att.obj == obj) return true;
    return false;
}


bool PinConstraint::OnInspectorGui(World* world)
{
    ImGui::Text("Type: Pin");
    bool changed = false;
    if (ImGui::Checkbox("Fixed X", &fixedX)) changed = true;
    ImGui::SameLine();
    if (ImGui::Checkbox("Fixed Y", &fixedY)) changed = true;
    
    if (ImGui::DragFloat2("Position", &position.x, 0.05f)) {
        for (auto& att : attachments) {
            Vec2 local = RotMatrix(-att.obj->transform.rotation).Rotate(position - att.obj->transform.position);
            att.localX = local.x;
            att.localY = local.y;
        }
        changed = true;
    }

    ImGui::Text("Attachments:");
    int toRemove = -1;
    for (size_t i = 0; i < attachments.size(); i++) {
        auto& att = attachments[i];
        ImGui::PushID(i);
        if (ImGui::Button("X")) toRemove = i;
        ImGui::SameLine();
        ImGui::BulletText("%s", att.obj->GetName().c_str());
        if (ImGui::DragFloat("Local X", &att.localX, 0.01f)) changed = true;
        if (ImGui::DragFloat("Local Y", &att.localY, 0.01f)) changed = true;
        ImGui::PopID();
    }

    if (toRemove != -1) {
        attachments.erase(attachments.begin() + toRemove);
        changed = true;
    }

    if (ImGui::Button("Add Attachment...")) {
        EditorState::Get().SetPickingMode(EditorState::PickingMode::CONSTRAINT_TARGET, ConstraintType::PIN, GetID());
    }

    return changed;
}