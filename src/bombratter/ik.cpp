#include "sm2d/colliders.h"
#include "sm2d/functions.h"
#include "sm2d/types.h"
#include <chrono>
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
        auto trans = engineState.scene.Get<Transform>(ent);

        // EntityID sen1 = engineState.scene.AddEntity();
        // engineState.scene.AssignParam<Name>(sen1, "Sen1");
        // auto trans1 = engineState.scene.AssignParam<Transform>(
        //     sen1, glm::vec3(ik->legRoot[0], 0.0f), glm::vec3(0.0f),
        //     glm::vec3(1.0f, 1.0f, 0.0f));
        // auto rigid =
        // engineState.scene.AssignParam<sm2d::Rigidbody>(
        //     sen1, sm2d::BodyType::sm2d_Static, trans1);
        // ik->groundSensor[0] =
        // engineState.scene.AssignParam<sm2d::Collider>(
        //     sen1, sm2d::ColliderType::sm2d_AABB,
        //     sm2d::ColAABB(glm::vec2(0.17f, 0.4f)), rigid, true);

        // EntityID sen2 = engineState.scene.AddEntity();
        // engineState.scene.AssignParam<Name>(sen2, "Sen2");
        // auto trans2 = engineState.scene.AssignParam<Transform>(
        //     sen2, glm::vec3(ik->legRoot[1], 0.0f), glm::vec3(0.0f),
        //     glm::vec3(1.0f, 1.0f, 0.0f));
        // auto rigid2 =
        // engineState.scene.AssignParam<sm2d::Rigidbody>(
        //     sen2, sm2d::BodyType::sm2d_Static, trans2);
        // ik->groundSensor[1] =
        // engineState.scene.AssignParam<sm2d::Collider>(
        //     sen2, sm2d::ColliderType::sm2d_AABB,
        //     sm2d::ColAABB(glm::vec2(0.17f, 0.4f)), rigid2, true);

        EntityID bodyEnt = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(bodyEnt, "BodyEnt");
        auto bodyEntTrans = engineState.scene.AssignParam<Transform>(
            bodyEnt, trans->position, glm::vec3(0.0f),
            glm::vec3(0.0f));
        ik->body = engineState.scene.AssignParam<sm2d::Rigidbody>(
            bodyEnt, sm2d::BodyType::sm2d_Dynamic, bodyEntTrans, 1.0f,
            true, 0.98f, 0.98f, 0.1f, true, 1.0f, 0, true, true);
        engineState.scene.AssignParam<sm2d::Collider>(
            bodyEnt, sm2d::ColliderType::sm2d_Circle,
            sm2d::ColCircle(0.1f), ik->body);

        EntityID headEnt = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(headEnt, "HeadEnt");
        auto headEntTrans = engineState.scene.AssignParam<Transform>(
            headEnt, trans->position + glm::vec3(0.0f, 0.3f, 0.0f),
            glm::vec3(0.0f), glm::vec3(0.0f));
        ik->head = engineState.scene.AssignParam<sm2d::Rigidbody>(
            headEnt, sm2d::BodyType::sm2d_Dynamic, headEntTrans, 1.0f,
            true, 0.98f, 0.98f, 0.1f, true, 1.0f, 0, false, true);
        engineState.scene.AssignParam<sm2d::Collider>(
            headEnt, sm2d::ColliderType::sm2d_Circle,
            sm2d::ColCircle(0.1f), ik->head);
    }
}

REGISTER_START_SYSTEM(PlayerIKStartSys);

std::string FPS;
auto        lastTime = std::chrono::high_resolution_clock::now();
int         frameCount = 0;
float       fps = 0.0f;

float MoveTowards(float current, float target, float maxDelta)
{
    if (std::abs(target - current) <= maxDelta)
        return target;
    return current + glm::sign(target - current) * maxDelta;
}

glm::vec3 SlerpVectors(const glm::vec3& start, const glm::vec3& end,
                       float t)
{
    float smoothT = t * t * (3.0f - 2.0f * t);

    // Linear interpolation between start and end points
    return glm::mix(start, end, smoothT);
}

float GetSmoothInterpolationTimer(float period = 2.0f)
{
    // Get current time
    double currentTime = glfwGetTime();

    // Create a cyclical timer that smoothly goes between 0 and 1
    float t = std::fmod(currentTime, period) / period;

    // Optional: apply smoothstep for more natural easing
    return t * t * (3.0f - 2.0f * t);
}

void PlayerIKSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        auto ik = engineState.scene.Get<PlayerIK>(ent);

        // sm2d::ApplySpringJoint(ik->head,
        //                        ik->body->transform->position +
        //                            glm::vec3(0.0f, 0.3f, 0.0f),
        //                        0.01f, 100.0f, 0.1f);

        ik->head->transform->position =
            SlerpVectors(ik->head->transform->position,
                         ik->body->transform->position +
                             glm::vec3(0.0f, 0.3f, 0.0f),
                         GetSmoothInterpolationTimer(1.0f));

        std::cout << "Interop: " << GetSmoothInterpolationTimer(1.0f) << '\n'; 
    }
}

REGISTER_SYSTEM(PlayerIKSys);

void PlayerIKDraw(PlayerIK* ik)
{
    if (ImGui::CollapsingHeader("PlayerIK"))
    {
        // ImGui::DragFloat2("legPos1",
        // glm::value_ptr(ik->legPos[0]));
        // ImGui::DragFloat2("legPos2",
        // glm::value_ptr(ik->legPos[1]));
        // ImGui::DragFloat2("handTarget1",
        // glm::value_ptr(ik->handTarget[0]));
        // ImGui::DragFloat2("handTarget2",
        // glm::value_ptr(ik->handTarget[1]));

        // ImGui::DragFloat2("legRoot1",
        // glm::value_ptr(ik->legRoot[0]));
        // ImGui::DragFloat2("legRoot2",
        // glm::value_ptr(ik->legRoot[1]));
        // ImGui::DragFloat2("handRoot1",
        // glm::value_ptr(ik->handRoot[0]));
        // ImGui::DragFloat2("handRoot2",
        // glm::value_ptr(ik->handRoot[1]));
        // ImGui::DragFloat("CircleCastRadius",
        // &ik->circleCastRadius); ImGui::DragFloat("LegThreshold",
        // &ik->legThreshold); ImGui::DragFloat("MaxSpeed",
        // &ik->maxSpeed); ImGui::DragFloat("Acceleration",
        // &ik->acceleration); ImGui::DragFloat("Deceleration",
        // &ik->deceleration); if (ImGui::DragFloat("HandElasticity",
        // &ik->handElasticity))
        // {
        //     ik->handRopeSim[0].elasticity = ik->handElasticity;
        //     ik->handRopeSim[1].elasticity = ik->handElasticity;
        // }

        // if (ImGui::DragFloat("HandPointDistance",
        // &ik->handPointDistance))
        // {
        //     ik->handRopeSim[0].pointDistance =
        //     ik->handPointDistance; ik->handRopeSim[1].pointDistance
        //     = ik->handPointDistance;
        // }

        // if (ImGui::DragInt("HandNumPoints", &ik->handNumPoints))
        // {
        //     ik->handRopeSim[0].numPoints = ik->handNumPoints;
        //     ik->handRopeSim[1].numPoints = ik->handNumPoints;
        // }

        // if (ImGui::DragFloat("HandDamping", &ik->handDamping))
        // {
        //     ik->handRopeSim[0].damping = ik->handDamping;
        //     ik->handRopeSim[1].damping = ik->handDamping;
        // }

        // if (ik->transform == nullptr)
        // {
        //     return;
        // }

        // glm::vec2 bodyPos = glm::vec2(ik->transform->position);

        // Renderer::RenderLine2D({ik->legRoot[0] + bodyPos,
        // ik->legPos[0]}, engineState.projMat,
        //                        engineState.camera->GetViewMatrix(),
        //                        glm::vec4(0.188235294f, 0.23137254f,
        //                        0.3176470f, 1.0f), 10.0f, 50.0f);
        // Renderer::RenderLine2D({ik->legRoot[1] + bodyPos,
        // ik->legPos[1]}, engineState.projMat,
        //                        engineState.camera->GetViewMatrix(),
        //                        glm::vec4(0.188235294f, 0.23137254f,
        //                        0.3176470f, 1.0f), 10.0f, 50.0f);

        // Renderer::RenderPoint(glm::vec3(bodyPos, 0.0f),
        // engineState.projMat,
        //                       engineState.camera->GetViewMatrix(),
        //                       glm::vec4(1.0f, 0.0f, 0.0f, 1.0));

        // Renderer::RenderLine2D(
        //     ik->handRopeSim[0].points, engineState.projMat,
        //     engineState.camera->GetViewMatrix(),
        //     glm::vec4(0.188235294f, 0.23137254f, 0.3176470f, 1.0f),
        //     0.1f, 50.0f, false);

        // Renderer::RenderLine2D(
        //     ik->handRopeSim[1].points, engineState.projMat,
        //     engineState.camera->GetViewMatrix(),
        //     glm::vec4(0.188235294f, 0.23137254f, 0.3176470f, 1.0f),
        //     0.1f, 50.0f, false);

        Renderer::RenderPoint(ik->head->transform->position,
                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        Renderer::RenderPoint(ik->body->transform->position,
                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        // Update FPS every second
        auto currentTime = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> elapsed = currentTime - lastTime;
        lastTime = currentTime;
        frameCount++;
        static float timeAccumulator = 0.0f;
        timeAccumulator += elapsed.count();
        if (timeAccumulator >= 0.1f)
        {
            fps = frameCount / timeAccumulator;

            // Reset counters
            frameCount = 0;
            timeAccumulator = 0.0f;

            // Update the FPS string
            FPS = std::to_string(fps);

            engineState.window->SetTitle(FPS.c_str());
        }
    }
}

nlohmann::json PlayerIKSave(PlayerIK* ik)
{
    nlohmann::json j;

    return j;
}

void PlayerIKLoad(PlayerIK* ik, const nlohmann::json& j)
{
    // if (j.contains("CircleCastRadius"))
    // {
    //     ik->circleCastRadius = j["CircleCastRadius"];
    // }
    // if (j.contains("LegThreshold"))
    // {
    //     ik->legThreshold = j["LegThreshold"];
    // }
    // if (j.contains("LegPos1"))
    // {
    //     ik->legPos[0] = {j["LegPos1"][0], j["LegPos1"][1]};
    // }
    // if (j.contains("LegPos2"))
    // {
    //     ik->legPos[1] = {j["LegPos2"][0], j["LegPos2"][1]};
    // }
    // if (j.contains("HandPos1"))
    // {
    //     ik->handTarget[0] = {j["HandPos1"][0], j["HandPos1"][1]};
    // }
    // if (j.contains("HandPos2"))
    // {
    //     ik->handTarget[1] = {j["HandPos2"][0], j["HandPos2"][1]};
    // }
    // if (j.contains("LegRoot1"))
    // {
    //     ik->legRoot[0] = {j["LegRoot1"][0], j["LegRoot1"][1]};
    // }
    // if (j.contains("LegRoot2"))
    // {
    //     ik->legRoot[1] = {j["LegRoot2"][0], j["LegRoot2"][1]};
    // }
    // if (j.contains("HandRoot1"))
    // {
    //     ik->handRoot[0] = {j["HandRoot1"][0], j["HandRoot1"][1]};
    // }
    // if (j.contains("HandRoot2"))
    // {
    //     ik->handRoot[1] = {j["HandRoot2"][0], j["HandRoot2"][1]};
    // }
    // if (j.contains("HandElasticity"))
    // {
    //     ik->handElasticity = j["HandElasticity"];
    // }
    // if (j.contains("HandPointDistance"))
    // {
    //     ik->handPointDistance = j["HandPointDistance"];
    // }
    // if (j.contains("HandDamping"))
    // {
    //     ik->handDamping = j["HandDamping"];
    // }
    // if (j.contains("HandNumPoints"))
    // {
    //     ik->handNumPoints = j["HandNumPoints"];
    // }
    // if (j.contains("MaxSpeed"))
    // {
    //     ik->maxSpeed = j["MaxSpeed"];
    // }
    // if (j.contains("Acceleration"))
    // {
    //     ik->acceleration = j["Acceleration"];
    // }
    // if (j.contains("Deceleration"))
    // {
    //     ik->deceleration = j["Deceleration"];
    // }
}

REGISTER_COMPONENT(PlayerIK, PlayerIKDraw, PlayerIKSave,
                   PlayerIKLoad);
