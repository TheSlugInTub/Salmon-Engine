#include "salmon/utils.h"
#include "sm2d/functions.h"
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

        EntityID face = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(face, "FaceSprite");
        engineState.scene.AssignParam<SpriteRenderer>(
            face, Utils::LoadTexture("res/textures/slug/face.png"));
        ik->faceTransform = engineState.scene.AssignParam<Transform>(
            face, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.3f, 0.3f, 0.0f));

        EntityID body = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(body, "BodySprite");
        engineState.scene.AssignParam<SpriteRenderer>(
            body, Utils::LoadTexture("res/textures/slug/body.png"));
        ik->bodyTransform = engineState.scene.AssignParam<Transform>(
            body, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.3f, 0.3f, 0.0f));

        ik->rigidbody = engineState.scene.Get<sm2d::Rigidbody>(ent);

        ik->transform = engineState.scene.Get<Transform>(ent);

        ik->handRopeSim[0] = RopeSim(ik->handRoot[0], ik->handNumPoints, ik->handElasticity,
                                     ik->handPointDistance, ik->handDamping);
        ik->handRopeSim[1] = RopeSim(ik->handRoot[1], ik->handNumPoints, ik->handElasticity,
                                     ik->handPointDistance, ik->handDamping);
    }
}

REGISTER_START_SYSTEM(PlayerIKStartSys);

std::string FPS;
auto        lastTime = std::chrono::high_resolution_clock::now();
int         frameCount = 0;
float       fps = 0.0f;

void PlayerIKSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        auto      ik = engineState.scene.Get<PlayerIK>(ent);
        glm::vec2 bodyPos = glm::vec2(ik->transform->position);
        glm::vec2 worldSpaceLegRoot[2] = {ik->legRoot[0] + bodyPos, ik->legRoot[1] + bodyPos};

        // unsigned chars to save on memory
        unsigned char leg1Moved = glm::fastDistance(ik->legPos[0], bodyPos) > ik->legThreshold;
        unsigned char leg2Moved = glm::fastDistance(ik->legPos[1], bodyPos) > ik->legThreshold;

        unsigned char isColliding1 = ik->groundSensor[0]->sensorCollider != nullptr;
        unsigned char isColliding2 = ik->groundSensor[1]->sensorCollider != nullptr;

        std::cout << "Is leg1 colliding? " << (int)isColliding1 << '\n';
        std::cout << "Is leg2 colliding? " << (int)isColliding2 << '\n';

        if (!isColliding1 && !isColliding2)
        {
            ik->legPos[0] = worldSpaceLegRoot[0] + glm::vec2(0.0f, -0.2f);
            ik->legPos[1] = worldSpaceLegRoot[1] + glm::vec2(0.0f, -0.2f);
        }
        else
        {
            if (isColliding1 && leg1Moved)
            {
                ik->legPos[0] = sm2d::FindClosestPointOnPolygon(
                    ik->groundSensor[0]->sensorCollider->polygon, worldSpaceLegRoot[0]);
            }

            if (isColliding2 && leg2Moved)
            {
                ik->legPos[1] = sm2d::FindClosestPointOnPolygon(
                    ik->groundSensor[1]->sensorCollider->polygon, worldSpaceLegRoot[1]);
            }

            if (Input::GetKeyDown(Key::Up))
            {
                ik->rigidbody->awake = true;
                ik->rigidbody->hasMoved = true;
                ik->rigidbody->force.y += 1400.0f;
            }
        }

        ik->groundSensor[0]->body->transform->position = glm::vec3(worldSpaceLegRoot[0], 0.0f);
        ik->groundSensor[1]->body->transform->position = glm::vec3(worldSpaceLegRoot[1], 0.0f);

        ik->faceTransform->position = glm::vec3(bodyPos + glm::vec2(0.0f, 0.2f), 0.0f);
        ik->bodyTransform->position = glm::vec3(bodyPos + glm::vec2(0.0f, -0.07f), 0.0f);

        ik->handRopeSim[0].basePosition = ik->handRoot[0] + bodyPos;
        ik->handRopeSim[0].points[0] = ik->handRoot[0] + bodyPos;
        ik->handRopeSim[0].prevPoints[0] = ik->handRoot[0] + bodyPos;
        ik->handRopeSim[1].basePosition = ik->handRoot[1] + bodyPos;
        ik->handRopeSim[1].points[0] = ik->handRoot[1] + bodyPos;
        ik->handRopeSim[1].prevPoints[0] = ik->handRoot[1] + bodyPos;

        SimulateRope(ik->handRopeSim[0]);
        SimulateRope(ik->handRopeSim[1]);

        if (Input::GetKey(Key::Left))
        {
            ik->rigidbody->awake = true;
            ik->rigidbody->hasMoved = true;
            ik->rigidbody->linearVelocity.x -= 0.05f;
        }
        else if (Input::GetKey(Key::Right))
        {
            ik->rigidbody->awake = true;
            ik->rigidbody->hasMoved = true;
            ik->rigidbody->linearVelocity.x += 0.05f;
        }
    }
}

REGISTER_SYSTEM(PlayerIKSys);

void PlayerIKDraw(PlayerIK* ik)
{
    if (ImGui::CollapsingHeader("PlayerIK"))
    {
        ImGui::DragFloat2("legPos1", glm::value_ptr(ik->legPos[0]));
        ImGui::DragFloat2("legPos2", glm::value_ptr(ik->legPos[1]));
        ImGui::DragFloat2("handTarget1", glm::value_ptr(ik->handTarget[0]));
        ImGui::DragFloat2("handTarget2", glm::value_ptr(ik->handTarget[1]));

        ImGui::DragFloat2("legRoot1", glm::value_ptr(ik->legRoot[0]));
        ImGui::DragFloat2("legRoot2", glm::value_ptr(ik->legRoot[1]));
        ImGui::DragFloat2("handRoot1", glm::value_ptr(ik->handRoot[0]));
        ImGui::DragFloat2("handRoot2", glm::value_ptr(ik->handRoot[1]));

        ImGui::DragFloat("CircleCastRadius", &ik->circleCastRadius);
        ImGui::DragFloat("LegThreshold", &ik->legThreshold);

        if (ImGui::DragFloat("HandElasticity", &ik->handElasticity))
        {
            ik->handRopeSim[0].elasticity = ik->handElasticity;
            ik->handRopeSim[1].elasticity = ik->handElasticity;
        }

        if (ImGui::DragFloat("HandPointDistance", &ik->handPointDistance))
        {
            ik->handRopeSim[0].pointDistance = ik->handPointDistance;
            ik->handRopeSim[1].pointDistance = ik->handPointDistance;
        }

        if (ImGui::DragInt("HandNumPoints", &ik->handNumPoints))
        {
            ik->handRopeSim[0].numPoints = ik->handNumPoints;
            ik->handRopeSim[1].numPoints = ik->handNumPoints;
        }

        if (ImGui::DragFloat("HandDamping", &ik->handDamping))
        {
            ik->handRopeSim[0].damping = ik->handDamping;
            ik->handRopeSim[1].damping = ik->handDamping;
        }

        if (ik->transform == nullptr)
        {
            return;
        }

        glm::vec2 bodyPos = glm::vec2(ik->transform->position);

        Renderer::RenderLine2D({ik->legRoot[0] + bodyPos, ik->legPos[0]}, engineState.projMat,
                               engineState.camera->GetViewMatrix(),
                               glm::vec4(0.188235294f, 0.23137254f, 0.3176470f, 1.0f), 10.0f,
                               50.0f);
        Renderer::RenderLine2D({ik->legRoot[1] + bodyPos, ik->legPos[1]}, engineState.projMat,
                               engineState.camera->GetViewMatrix(),
                               glm::vec4(0.188235294f, 0.23137254f, 0.3176470f, 1.0f), 10.0f,
                               50.0f);

        Renderer::RenderPoint(glm::vec3(bodyPos, 0.0f), engineState.projMat,
                              engineState.camera->GetViewMatrix(),
                              glm::vec4(1.0f, 0.0f, 0.0f, 1.0));

        Renderer::RenderLine2D(
            ik->handRopeSim[0].points, engineState.projMat, engineState.camera->GetViewMatrix(),
            glm::vec4(0.188235294f, 0.23137254f, 0.3176470f, 1.0f), 0.1f, 50.0f, false);

        Renderer::RenderLine2D(
            ik->handRopeSim[1].points, engineState.projMat, engineState.camera->GetViewMatrix(),
            glm::vec4(0.188235294f, 0.23137254f, 0.3176470f, 1.0f), 0.1f, 50.0f, false);

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
    nlohmann::json j = {{"CircleCastRadius", ik->circleCastRadius},
                        {"LegThreshold", ik->legThreshold},
                        {"LegPos1", {ik->legPos[0].x, ik->legPos[0].y}},
                        {"LegPos2", {ik->legPos[1].x, ik->legPos[1].y}},
                        {"HandPos1", {ik->handTarget[0].x, ik->handTarget[0].y}},
                        {"HandPos2", {ik->handTarget[1].x, ik->handTarget[1].y}},
                        {"LegRoot1", {ik->legRoot[0].x, ik->legRoot[0].y}},
                        {"LegRoot2", {ik->legRoot[1].x, ik->legRoot[1].y}},
                        {"HandElasticity", ik->handElasticity},
                        {"HandPointDistance", ik->handPointDistance},
                        {"HandDamping", ik->handDamping},
                        {"HandNumPoints", ik->handNumPoints},
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
        ik->handTarget[0] = {j["HandPos1"][0], j["HandPos1"][1]};
    }
    if (j.contains("HandPos2"))
    {
        ik->handTarget[1] = {j["HandPos2"][0], j["HandPos2"][1]};
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
    if (j.contains("HandElasticity"))
    {
        ik->handElasticity = j["HandElasticity"];
    }
    if (j.contains("HandPointDistance"))
    {
        ik->handPointDistance = j["HandPointDistance"];
    }
    if (j.contains("HandDamping"))
    {
        ik->handDamping = j["HandDamping"];
    }
    if (j.contains("HandNumPoints"))
    {
        ik->handNumPoints = j["HandNumPoints"];
    }
}

REGISTER_COMPONENT(PlayerIK, PlayerIKDraw, PlayerIKSave, PlayerIKLoad);
