#include <chrono>
#include <salmon/editor.h>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <bombratter/ik.h>
#include <salmon/json.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/string_cast.hpp>
#include <salmon/ik_solver.h>
#include <sm2d/functions.h>

void PlayerIKStartSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        auto ik = engineState.scene.Get<PlayerIK>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);

        EntityID sen1 = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(sen1, "Sen1");

        auto trans1 = engineState.scene.AssignParam<Transform>(
            sen1, glm::vec3(ik->legRoot[0], 0.0f), glm::vec3(0.0f),
            glm::vec3(1.0f, 1.0f, 0.0f));

        auto rigid = engineState.scene.AssignParam<sm2d::Rigidbody>(
            sen1, sm2d::BodyType::sm2d_Static, trans1);

        ik->groundSensor[0] =
            engineState.scene.AssignParam<sm2d::Collider>(
                sen1, sm2d::ColliderType::sm2d_AABB,
                sm2d::ColAABB(glm::vec2(0.17f, 0.2f)), rigid, true);

        EntityID sen2 = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(sen2, "Sen2");

        auto trans2 = engineState.scene.AssignParam<Transform>(
            sen2, glm::vec3(ik->legRoot[1], 0.0f), glm::vec3(0.0f),
            glm::vec3(1.0f, 1.0f, 0.0f));

        auto rigid2 = engineState.scene.AssignParam<sm2d::Rigidbody>(
            sen2, sm2d::BodyType::sm2d_Static, trans2);

        ik->groundSensor[1] =
            engineState.scene.AssignParam<sm2d::Collider>(
                sen2, sm2d::ColliderType::sm2d_AABB,
                sm2d::ColAABB(glm::vec2(0.17f, 0.2f)), rigid2, true);

        // -----

        for (int i = 0; i < 2; i++)
        {
            EntityID bodyEnt = engineState.scene.AddEntity();
            engineState.scene.AssignParam<Name>(
                bodyEnt, "BodyEnt" + std::to_string(i));

            auto bodyEntTrans =
                engineState.scene.AssignParam<Transform>(
                    bodyEnt, trans->position, glm::vec3(0.0f),
                    glm::vec3(0.0f));

            ik->body[i] =
                engineState.scene.AssignParam<sm2d::Rigidbody>(
                    bodyEnt, sm2d::BodyType::sm2d_Dynamic,
                    bodyEntTrans, 0.1f, true, 0.98f, 0.98f, 0.1f,
                    true, 1.0f, 255, false, true);

            engineState.scene.AssignParam<sm2d::Collider>(
                bodyEnt, sm2d::ColliderType::sm2d_Circle,
                sm2d::ColCircle(0.05f), ik->body[i]);
        }

        EntityID headEnt = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(headEnt, "HeadEnt");

        auto headEntTrans = engineState.scene.AssignParam<Transform>(
            headEnt, trans->position + glm::vec3(0.0f, 0.3f, 0.0f),
            glm::vec3(0.0f), glm::vec3(0.0f));

        ik->head = engineState.scene.AssignParam<sm2d::Rigidbody>(
            headEnt, sm2d::BodyType::sm2d_Dynamic, headEntTrans, 1.0f,
            true, 0.98f, 0.98f, 0.1f, true, 1.0f, 255, false, true);

        engineState.scene.AssignParam<sm2d::Collider>(
            headEnt, sm2d::ColliderType::sm2d_Circle,
            sm2d::ColCircle(0.07f), ik->head);

        EntityID legEnt = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(legEnt, "LegEnt");

        auto legEntTrans = engineState.scene.AssignParam<Transform>(
            legEnt, trans->position + glm::vec3(0.0f, -0.2f, 0.0f),
            glm::vec3(0.0f), glm::vec3(0.0f));

        auto legEntBody =
            engineState.scene.AssignParam<sm2d::Rigidbody>(
                legEnt, sm2d::BodyType::sm2d_Dynamic, legEntTrans,
                1.0f, true, 0.98f, 0.98f, 0.1f, true, 1.0f, 255, true,
                true);

        ik->legCollider =
            engineState.scene.AssignParam<sm2d::Collider>(
                legEnt, sm2d::ColliderType::sm2d_Circle,
                sm2d::ColCircle(0.1f), legEntBody);

        ik->legIK[0] = IKSolver2D(ik->legRoot[0], glm::vec2(0.0f), 3,
                                  ik->legLength);
        ik->legIK[1] = IKSolver2D(ik->legRoot[1], glm::vec2(0.0f), 3,
                                  ik->legLength);
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
    float t = (float)(std::fmod(currentTime, period) / period);

    // Optional: apply smoothstep for more natural easing
    return t * t * (3.0f - 2.0f * t);
}

void PlayerIKSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        auto ik = engineState.scene.Get<PlayerIK>(ent);

        glm::vec2 worldSpaceLegRoot[2] = {
            glm::vec2(ik->legCollider->body->transform->position) +
                ik->legRoot[0],
            glm::vec2(ik->legCollider->body->transform->position) +
                ik->legRoot[1]};

        glm::vec2 bodyPos =
            ik->legCollider->body->transform->position;

        sm2d::ApplySpringJointWithinAngle(
            ik->head,
            ik->body[1]->transform->position +
                glm::vec3(0.0f, 0.2f, 0.0f),
            0.01f, 160.0f, 0.0f, 140, 40);
        sm2d::ApplySpringJointWithinAngle(
            ik->body[1],
            ik->body[0]->transform->position +
                glm::vec3(0.0f, 0.18f, 0.0f),
            0.01f, 160.0f, 0.0f, 140, 40);
        sm2d::ApplySpringJointWithinAngle(
            ik->body[0],
            ik->legCollider->body->transform->position +
                glm::vec3(0.0f, 0.25f, 0.0f),
            0.01f, 160.0f, 0.0f, 140, 40);

        // Check if legs need to move
        unsigned char leg1Moved =
            glm::distance(ik->legIK[0].endpoint,
                          worldSpaceLegRoot[0]) > ik->legThreshold;
        unsigned char leg2Moved =
            glm::distance(ik->legIK[1].endpoint,
                          worldSpaceLegRoot[1]) > ik->legThreshold;

        unsigned char notNull1 =
            ik->groundSensor[0]->sensorCollider != nullptr;
        unsigned char notNull2 =
            ik->groundSensor[1]->sensorCollider != nullptr;

        static bool  leg1CanMove = true;
        static float legMoveTimer = 0.0f;
        const float  LEG_MOVE_DELAY = 0.1f;

        legMoveTimer += engineState.deltaTime;

        static float multiplier = 0.0f;

        // Handle leg movement with alternation
        if (legMoveTimer >= LEG_MOVE_DELAY)
        {
            if (leg1Moved && notNull1 && leg1CanMove)
            {
                ResetIK2D(ik->legIK[0], glm::vec2(multiplier, 0.0f));
                ik->legIK[0].endpoint =
                    sm2d::FindClosestPointOnPolygon(
                        ik->groundSensor[0]->sensorCollider->polygon,
                        worldSpaceLegRoot[0] +
                            glm::vec2(0.1f * multiplier, 0.0f));
                leg1CanMove = false;
                legMoveTimer = 0.0f;
            }
            else if (leg2Moved && notNull2 && !leg1CanMove)
            {
                ResetIK2D(ik->legIK[1], glm::vec2(multiplier, 0.0f));
                ik->legIK[1].endpoint =
                    sm2d::FindClosestPointOnPolygon(
                        ik->groundSensor[1]->sensorCollider->polygon,
                        worldSpaceLegRoot[1] +
                            glm::vec2(0.1f * multiplier, 0.0f));
                leg1CanMove = true;
                legMoveTimer = 0.0f;
            }
        }

        if (notNull1 && notNull2)
        {
            if (Input::GetKeyDown(Key::Up))
            {
                ik->legCollider->body->force.y += 230.0f;
            }
        }

        ik->legIK[0].points[0] = worldSpaceLegRoot[0];
        ik->legIK[1].points[0] = worldSpaceLegRoot[1];

        SolveIK2D(ik->legIK[0]);
        SolveIK2D(ik->legIK[1]);

        ik->groundSensor[0]->body->transform->position =
            glm::vec3(worldSpaceLegRoot[0] - glm::vec2(0.0f, 0.2f), 0.0f);
        ik->groundSensor[1]->body->transform->position =
            glm::vec3(worldSpaceLegRoot[1] - glm::vec2(0.0f, 0.2f), 0.0f);

        if (Input::GetKey(Key::Left))
        {
            // Apply acceleration towards target speed
            multiplier = -1.0f;
            float targetSpeed = -ik->maxSpeed;
            float currentSpeed =
                ik->legCollider->body->linearVelocity.x;
            float acceleration =
                ik->acceleration * engineState.deltaTime;

            ik->legCollider->body->linearVelocity.x =
                MoveTowards(currentSpeed, targetSpeed, acceleration);
        }
        else if (Input::GetKey(Key::Right))
        {
            multiplier = 1.0f;
            float targetSpeed = ik->maxSpeed;
            float currentSpeed =
                ik->legCollider->body->linearVelocity.x;
            float acceleration =
                ik->acceleration * engineState.deltaTime;

            ik->legCollider->body->linearVelocity.x =
                MoveTowards(currentSpeed, targetSpeed, acceleration);
        }
        else
        {
            // Decelerate when no input
            float deceleration =
                ik->deceleration * engineState.deltaTime;
            ik->legCollider->body->linearVelocity.x =
                MoveTowards(ik->legCollider->body->linearVelocity.x,
                            0.0f, deceleration);
        }

        Renderer::RenderLine2D(ik->legIK[0].points,
                               glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
                               10.0f, 6.0f, false);
        Renderer::RenderLine2D(ik->legIK[1].points,
                               glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
                               10.0f, 6.0f, false);
    }
}

REGISTER_SYSTEM(PlayerIKSys);

void PlayerIKDraw(PlayerIK* ik)
{
    if (ImGui::CollapsingHeader("PlayerIK"))
    {
        ImGui::DragFloat2("LegRoot1", glm::value_ptr(ik->legRoot[0]));
        ImGui::DragFloat2("LegRoot2", glm::value_ptr(ik->legRoot[1]));
        ImGui::DragFloat("LegLength", &ik->legLength);
        ImGui::DragFloat("LegThreshold", &ik->legThreshold);

        ImGui::DragFloat("Acceleration", &ik->acceleration);
        ImGui::DragFloat("Deceleration", &ik->deceleration);
        ImGui::DragFloat("MaxSpeed", &ik->maxSpeed);

        // for (auto point : testIK.points)
        // {
        //     std::cout << "Point in points: " <<
        //     glm::to_string(point) << '\n';
        // }

        // Renderer::RenderLine2D(
        //     ik->handRopeSim[1].points, engineState.projMat,
        //     engineState.camera->GetViewMatrix(),
        //     glm::vec4(0.188235294f, 0.23137254f, 0.3176470f, 1.0f),
        //     0.1f, 50.0f, false);

        // Renderer::RenderPoint(ik->head->transform->position,
        //                       glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        // Renderer::RenderPoint(ik->body->transform->position,
        //                       glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

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
    nlohmann::json j = {
        {"LegRoot1", {ik->legRoot[0].x, ik->legRoot[0].y}},
        {"LegRoot2", {ik->legRoot[1].x, ik->legRoot[1].y}},
        {"LegThreshold", ik->legThreshold},
        {"LegLength", ik->legLength},
        {"Acceleration", ik->acceleration},
        {"MaxSpeed", ik->maxSpeed},
        {"Deceleration", ik->deceleration}};

    return j;
}

void PlayerIKLoad(PlayerIK* ik, const nlohmann::json& j)
{
    if (j.contains("LegLength"))
        ik->legLength = j["LegLength"];
    if (j.contains("LegRoot1"))
        ik->legRoot[0] = {j["LegRoot1"][0], j["LegRoot1"][1]};
    if (j.contains("LegRoot2"))
        ik->legRoot[1] = {j["LegRoot2"][0], j["LegRoot2"][1]};
    if (j.contains("LegThreshold"))
        ik->legThreshold = j["LegThreshold"];
    if (j.contains("Acceleration"))
        ik->acceleration = j["Acceleration"];
    if (j.contains("Deceleration"))
        ik->deceleration = j["Deceleration"];
    if (j.contains("MaxSpeed"))
        ik->maxSpeed = j["MaxSpeed"];
}

REGISTER_COMPONENT(PlayerIK, PlayerIKDraw, PlayerIKSave,
                   PlayerIKLoad);
