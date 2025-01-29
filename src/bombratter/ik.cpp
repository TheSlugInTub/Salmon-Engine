#include <salmon/editor.h>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <bombratter/ik.h>
#include <salmon/json.hpp>

void PlayerIKStartSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        
    }
}

REGISTER_START_SYSTEM(PlayerIKStartSys);

void PlayerIKSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        
    }
}

REGISTER_SYSTEM(PlayerIKSys);

void PlayerIKDraw(PlayerIK* ik)
{
    if (ImGui::CollapsingHeader("PlayerIK"))
    {
        ImGui::DragFloat2("faceTarget", glm::value_ptr(ik->faceTarget));
        ImGui::DragFloat2("bodyTarget", glm::value_ptr(ik->bodyTarget));
        
        ImGui::DragFloat2("legTarget1", glm::value_ptr(ik->legTarget[0]));
        ImGui::DragFloat2("legTarget2", glm::value_ptr(ik->legTarget[1]));
        ImGui::DragFloat2("handTarget1", glm::value_ptr(ik->handTarget[0]));
        ImGui::DragFloat2("handTarget2", glm::value_ptr(ik->handTarget[1]));
        
        ImGui::DragFloat2("legRoot1", glm::value_ptr(ik->legRoot[0]));
        ImGui::DragFloat2("legRoot2", glm::value_ptr(ik->legRoot[1]));
        ImGui::DragFloat2("handRoot1", glm::value_ptr(ik->handRoot[0]));
        ImGui::DragFloat2("handRoot2", glm::value_ptr(ik->handRoot[1]));
    }
}

nlohmann::json PlayerIKSave(PlayerIK* ik)
{
    nlohmann::json j = {};

    return j;
}

void PlayerIKLoad(PlayerIK* ik, const nlohmann::json& j)
{
    return;
}

REGISTER_COMPONENT(PlayerIK, PlayerIKDraw, PlayerIKSave, PlayerIKLoad);
