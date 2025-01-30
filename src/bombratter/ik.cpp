#include "sm2d/types.h"
#include <salmon/editor.h>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <bombratter/ik.h>
#include <salmon/json.hpp>

void PlayerIKStartSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        auto ik = engineState.scene.Get<PlayerIK>(ent);

        EntityID sen1 = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(sen1, "Sen1");
        auto     trans1 = engineState.scene.AssignParam<Transform>(
            sen1, glm::vec3(ik->legTarget[0], 0.0f), glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 0.0f));
        auto rigid = engineState.scene.AssignParam<sm2d::Rigidbody>(
            sen1, sm2d::BodyType::sm2d_Static, trans1);
        ik->groundSensor[0] = engineState.scene.AssignParam<sm2d::Collider>(
            sen1, sm2d::ColliderType::sm2d_Circle, sm2d::ColCircle(ik->circleCastRadius), rigid,
            true);

        EntityID sen2 = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(sen2, "Sen2");
        auto     trans2 = engineState.scene.AssignParam<Transform>(
            sen2, glm::vec3(ik->legTarget[1], 0.0f), glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 0.0f));
        auto rigid2 = engineState.scene.AssignParam<sm2d::Rigidbody>(
            sen2, sm2d::BodyType::sm2d_Static, trans2);
        ik->groundSensor[1] = engineState.scene.AssignParam<sm2d::Collider>(
            sen2, sm2d::ColliderType::sm2d_Circle, sm2d::ColCircle(ik->circleCastRadius), rigid2,
            true);
    }
}

REGISTER_START_SYSTEM(PlayerIKStartSys);

void PlayerIKSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        auto ik = engineState.scene.Get<PlayerIK>(ent);

        Renderer::RenderPoint(glm::vec3(ik->faceTarget, 0.0f), engineState.projMat,
                              engineState.camera->GetViewMatrix(),
                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
        Renderer::RenderPoint(glm::vec3(ik->bodyTarget, 0.0f), engineState.projMat,
                              engineState.camera->GetViewMatrix(),
                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
        Renderer::RenderPoint(glm::vec3(ik->legTarget[0], 0.0f), engineState.projMat,
                              engineState.camera->GetViewMatrix(),
                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
        Renderer::RenderPoint(glm::vec3(ik->legTarget[1], 0.0f), engineState.projMat,
                              engineState.camera->GetViewMatrix(),
                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
        Renderer::RenderPoint(glm::vec3(ik->handTarget[0], 0.0f), engineState.projMat,
                              engineState.camera->GetViewMatrix(),
                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
        Renderer::RenderPoint(glm::vec3(ik->handTarget[1], 0.0f), engineState.projMat,
                              engineState.camera->GetViewMatrix(),
                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
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

        ImGui::DragFloat("CircleCastRadius", &ik->circleCastRadius);
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
