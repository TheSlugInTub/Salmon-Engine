#include "sm2d/functions.h"
#include <salmon/editor.h>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <bombratter/ik.h>
#include <salmon/json.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/string_cast.hpp>

void PlayerIKStartSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        auto ik = engineState.scene.Get<PlayerIK>(ent);

        EntityID sen1 = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(sen1, "Sen1");
        auto trans1 = engineState.scene.AssignParam<Transform>(
            sen1, glm::vec3(ik->legPos[0], 0.0f), glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 0.0f));
        auto rigid = engineState.scene.AssignParam<sm2d::Rigidbody>(
            sen1, sm2d::BodyType::sm2d_Static, trans1);
        ik->groundSensor[0] = engineState.scene.AssignParam<sm2d::Collider>(
            sen1, sm2d::ColliderType::sm2d_AABB, sm2d::ColAABB(glm::vec2(0.5f, 0.5f)), rigid, true);

        EntityID sen2 = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(sen2, "Sen2");
        auto trans2 = engineState.scene.AssignParam<Transform>(
            sen2, glm::vec3(ik->legPos[1], 0.0f), glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 0.0f));
        auto rigid2 = engineState.scene.AssignParam<sm2d::Rigidbody>(
            sen2, sm2d::BodyType::sm2d_Static, trans2);
        ik->groundSensor[1] = engineState.scene.AssignParam<sm2d::Collider>(
            sen2, sm2d::ColliderType::sm2d_AABB, sm2d::ColAABB(glm::vec2(0.5f, 0.5f)), rigid2,
            true);
    }
}

REGISTER_START_SYSTEM(PlayerIKStartSys);

void PlayerIKSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        auto ik = engineState.scene.Get<PlayerIK>(ent);

        if (glm::fastDistance(ik->legPos[0], ik->bodyPos) > ik->legThreshold)
        {
            if (ik->groundSensor[0]->colliding)
            {
                ik->legPos[0] = sm2d::FindClosestPointOnPolygon(
                    ik->groundSensor[0]->sensorCollider->polygon, ik->legRoot[0] + ik->bodyPos);
            }
        }

        if (glm::fastDistance(ik->legPos[1], ik->bodyPos) > ik->legThreshold)
        {
            if (ik->groundSensor[1]->colliding)
            {
                ik->legPos[1] = sm2d::FindClosestPointOnPolygon(
                    ik->groundSensor[1]->sensorCollider->polygon, ik->legRoot[1] + ik->bodyPos);
            }
        }

        if (!ik->groundSensor[0]->colliding && !ik->groundSensor[1]->colliding)
        {
            ik->legPos[0] = ik->legRoot[0] + glm::vec2(0.0f, -0.3f) + ik->bodyPos;
            ik->legPos[1] = ik->legRoot[1] + glm::vec2(0.0f, -0.3f) + ik->bodyPos;
        }

        ik->groundSensor[0]->body->transform->position =
            glm::vec3(ik->legRoot[0] + ik->bodyPos, 0.0f);
        ik->groundSensor[1]->body->transform->position =
            glm::vec3(ik->legRoot[1] + ik->bodyPos, 0.0f);
    }
}

REGISTER_SYSTEM(PlayerIKSys);

void PlayerIKDraw(PlayerIK* ik)
{
    if (ImGui::CollapsingHeader("PlayerIK"))
    {
        ImGui::DragFloat2("facePos", glm::value_ptr(ik->facePos));
        ImGui::DragFloat2("bodyPos", glm::value_ptr(ik->bodyPos));

        ImGui::DragFloat2("legPos1", glm::value_ptr(ik->legPos[0]));
        ImGui::DragFloat2("legPos2", glm::value_ptr(ik->legPos[1]));
        ImGui::DragFloat2("handPos1", glm::value_ptr(ik->handPos[0]));
        ImGui::DragFloat2("handPos2", glm::value_ptr(ik->handPos[1]));

        ImGui::DragFloat2("legRoot1", glm::value_ptr(ik->legRoot[0]));
        ImGui::DragFloat2("legRoot2", glm::value_ptr(ik->legRoot[1]));
        ImGui::DragFloat2("handRoot1", glm::value_ptr(ik->handRoot[0]));
        ImGui::DragFloat2("handRoot2", glm::value_ptr(ik->handRoot[1]));

        ImGui::DragFloat("CircleCastRadius", &ik->circleCastRadius);
        ImGui::DragFloat("LegThreshold", &ik->legThreshold);

        Renderer::RenderLine(
            {glm::vec3(ik->legRoot[0] + ik->bodyPos, 0.0f), glm::vec3(ik->legPos[0], 0.0f)},
            engineState.projMat, engineState.camera->GetViewMatrix(),
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
        Renderer::RenderLine(
            {glm::vec3(ik->legRoot[1] + ik->bodyPos, 0.0f), glm::vec3(ik->legPos[1], 0.0f)},
            engineState.projMat, engineState.camera->GetViewMatrix(),
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0));

        Renderer::RenderPoint(glm::vec3(ik->bodyPos, 0.0f), engineState.projMat,
                              engineState.camera->GetViewMatrix(),
                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
    }
}

nlohmann::json PlayerIKSave(PlayerIK* ik)
{
    nlohmann::json j = {{"CircleCastRadius", ik->circleCastRadius},
                        {"LegThreshold", ik->legThreshold},
                        {"FacePos", {ik->facePos.x, ik->facePos.y}},
                        {"BodyPos", {ik->bodyPos.x, ik->bodyPos.y}},
                        {"LegPos1", {ik->legPos[0].x, ik->legPos[0].y}},
                        {"LegPos2", {ik->legPos[1].x, ik->legPos[1].y}},
                        {"HandPos1", {ik->handPos[0].x, ik->handPos[0].y}},
                        {"HandPos2", {ik->handPos[1].x, ik->handPos[1].y}},
                        {"LegRoot1", {ik->legRoot[0].x, ik->legRoot[0].y}},
                        {"LegRoot2", {ik->legRoot[1].x, ik->legRoot[1].y}},
                        {"HandRoot1", {ik->handRoot[0].x, ik->handRoot[0].y}},
                        {"HandRoot2", {ik->handRoot[1].x, ik->handRoot[1].y}}};

    return j;
}

void PlayerIKLoad(PlayerIK* ik, const nlohmann::json& j)
{
    if (j.contains("CircleCastRadius"))
    {
        ik->circleCastRadius = j["CircleCastRadius"];
    }
    if (j.contains("LegThreshold"))
    {
        ik->legThreshold = j["LegThreshold"];
    }
    if (j.contains("FacePos"))
    {
        ik->facePos = {j["FacePos"][0], j["FacePos"][1]};
    }
    if (j.contains("BodyPos"))
    {
        ik->bodyPos = {j["BodyPos"][0], j["BodyPos"][1]};
    }
    if (j.contains("LegPos1"))
    {
        ik->legPos[0] = {j["LegPos1"][0], j["LegPos1"][1]};
    }
    if (j.contains("LegPos2"))
    {
        ik->legPos[1] = {j["LegPos2"][0], j["LegPos2"][1]};
    }
    if (j.contains("HandPos1"))
    {
        ik->handPos[0] = {j["HandPos1"][0], j["HandPos1"][1]};
    }
    if (j.contains("HandPos2"))
    {
        ik->handPos[1] = {j["HandPos2"][0], j["HandPos2"][1]};
    }
    if (j.contains("LegRoot1"))
    {
        ik->legRoot[0] = {j["LegRoot1"][0], j["LegRoot1"][1]};
    }
    if (j.contains("LegRoot2"))
    {
        ik->legRoot[1] = {j["LegRoot2"][0], j["LegRoot2"][1]};
    }
    if (j.contains("HandRoot1"))
    {
        ik->handRoot[0] = {j["HandRoot1"][0], j["HandRoot1"][1]};
    }
    if (j.contains("HandRoot2"))
    {
        ik->handRoot[1] = {j["HandRoot2"][0], j["HandRoot2"][1]};
    }
}

REGISTER_COMPONENT(PlayerIK, PlayerIKDraw, PlayerIKSave, PlayerIKLoad);
