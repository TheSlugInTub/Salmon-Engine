#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <salmon/editor.h>
#include <salmon/components.h>

void TransformDraw(Transform* trans)
{
    if (ImGui::CollapsingHeader("Transform"))
    {
        ImGui::DragFloat3("Position", glm::value_ptr(trans->position));
        ImGui::DragFloat3("Rotation", glm::value_ptr(trans->rotation));
        ImGui::DragFloat3("Scale", glm::value_ptr(trans->scale));
    }
}

nlohmann::json TransformSave(Transform* trans)
{
    return 
    {
        {"Position", {trans->position.x, trans->position.y, trans->position.z}},
        {"Rotation", {trans->rotation.x, trans->rotation.y, trans->rotation.z}},
        {"Scale", {trans->scale.x, trans->scale.y, trans->scale.z}}
    };
}

REGISTER_COMPONENT(Transform, TransformDraw, TransformSave);
