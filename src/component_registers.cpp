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

REGISTER_DRAW(Transform, TransformDraw);
