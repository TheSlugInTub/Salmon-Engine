#include <salmon/editor.h>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <bombratter/ik.h>
#include <salmon/json.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/string_cast.hpp>
#include <salmon/ik_solver.h>
#include <sm2d/functions.h>
#include <bombratter/background_sprite.h>

ShapeCastContext2D context;
bool               itemInReach = false;

float HandCastCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal,
                       float fraction, void* context)
{
    itemInReach = true;

    ShapeCastContext2D* myContext = (ShapeCastContext2D*)context;
    myContext->collidingShape = shapeId;
    myContext->collidingBody = b2Shape_GetBody(shapeId);

    return fraction;
}

void PlayerIKStartSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        auto ik = engineState.scene.Get<PlayerIK>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);

        EntityID legEnt = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(legEnt, "LegEnt");

        auto legEntTrans = engineState.scene.AssignParam<Transform>(
            legEnt, trans->position + glm::vec3(0.0f, -0.2f, 0.0f),
            glm::vec3(0.0f), glm::vec3(0.0f));

        auto legEntBody = engineState.scene.AssignParam<Rigidbody2D>(
            legEnt, rg2d_Dynamic, legEntTrans, 1.0f, 1.0f, 0.1f, 0.1f,
            0.1f, false, true, 255);

        ik->legCollider = engineState.scene.AssignParam<Collider2D>(
            legEnt, legEntBody, 0.08f, col_Player);

        ik->legIK[0] = IKSolver2D(ik->legRoot[0], glm::vec2(0.0f), 3,
                                  ik->legLength);
        ik->legIK[1] = IKSolver2D(ik->legRoot[1], glm::vec2(0.0f), 3,
                                  ik->legLength);

        ik->handIK[0] = IKSolver2D(ik->handRoot[0], glm::vec2(0.0f),
                                   3, ik->handLength);
        ik->handIK[1] = IKSolver2D(ik->handRoot[1], glm::vec2(0.0f),
                                   3, ik->handLength);

        for (int i = 0; i < 2; i++)
        {
            EntityID bodyEnt = engineState.scene.AddEntity();
            engineState.scene.AssignParam<Name>(
                bodyEnt, "BodyEnt" + std::to_string(i));

            auto bodyEntTrans =
                engineState.scene.AssignParam<Transform>(
                    bodyEnt, trans->position, glm::vec3(0.0f),
                    glm::vec3(0.2f));

            ik->body[i] = engineState.scene.AssignParam<Rigidbody2D>(
                bodyEnt, rg2d_Dynamic, bodyEntTrans, 1.0f, 1.0f, 0.1f,
                0.1f, 0.1f, false, true, 255);

            engineState.scene.AssignParam<Collider2D>(
                bodyEnt, ik->body[i], 0.05f, col_Player);

            engineState.scene.AssignParam<SpriteRenderer>(
                bodyEnt,
                Utils::LoadTexture("res/textures/slug/body1.png"));
        }

        EntityID headEnt = engineState.scene.AddEntity();
        engineState.scene.AssignParam<Name>(headEnt, "HeadEnt");

        auto headEntTrans = engineState.scene.AssignParam<Transform>(
            headEnt, trans->position + glm::vec3(0.0f, 0.3f, 0.0f),
            glm::vec3(0.0f), glm::vec3(0.2f));

        ik->head = engineState.scene.AssignParam<Rigidbody2D>(
            headEnt, rg2d_Dynamic, headEntTrans, 1.0f, 1.0f, 0.1f,
            0.1f, 0.1f, false, true, 255);

        engineState.scene.AssignParam<Collider2D>(headEnt, ik->head,
                                                  0.07f, col_Player);

        engineState.scene.AssignParam<SpriteRenderer>(
            headEnt,
            Utils::LoadTexture("res/textures/slug/head.png"));

        EntityID eyesEnt = engineState.scene.AddEntity();

        ik->eyesTexture =
            Utils::LoadTexture("res/textures/slug/eyes.png");
        ik->closedEyesTexture =
            Utils::LoadTexture("res/textures/slug/closed_eyes.png");

        engineState.scene.AssignParam<Name>(eyesEnt, "EyesEnt");
        ik->eyesTransform = engineState.scene.AssignParam<Transform>(
            eyesEnt, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f),
            glm::vec3(0.25f, 0.25f, 0.0f));
        ik->eyes = engineState.scene.AssignParam<SpriteRenderer>(
            eyesEnt, ik->eyesTexture);

        Rigidbody2DStartSys();
        Collider2DStartSys();
        Rigidbody2DFixCollidersStartSys();

        CreateRevoluteJoint(
            ik->revoluteJoints[0], glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 3.0f / 2), ik->body[0]->bodyID,
            ik->body[1]->bodyID, -0.25f * B2_PI, 0.25f * B2_PI);
        CreateRevoluteJoint(
            ik->revoluteJoints[1], glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 3.0f / 2), ik->body[1]->bodyID,
            ik->head->bodyID, -0.25f * B2_PI, 0.25f * B2_PI);
        
        CreateDistanceJoint(
            ik->distanceJoints[0], glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 3.0f / 2), ik->body[0]->bodyID,
            ik->body[1]->bodyID, 0.5f);
        CreateDistanceJoint(
            ik->distanceJoints[1], glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 3.0f / 2), ik->head->bodyID,
            ik->head->bodyID, 0.5f);
    }
}

REGISTER_START_SYSTEM(PlayerIKStartSys);

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

float sine = sin(1.5708);
float cosine = cos(1.5708);

static float multiplier = 0.0f;

void PlayerIKSys()
{
    for (EntityID ent : SceneView<PlayerIK>(engineState.scene))
    {
        auto ik = engineState.scene.Get<PlayerIK>(ent);

        glm::vec2 bodyPos =
            glm::vec2(ik->legCollider->body->transform->position);

        glm::vec2 worldSpaceLegRoot[2];
        if (ik->state == PlayerState::Walking)
        {
            worldSpaceLegRoot[0] = bodyPos + ik->legRoot[0];
            worldSpaceLegRoot[1] = bodyPos + ik->legRoot[1];
        }
        else
        {
            worldSpaceLegRoot[0] = bodyPos + ik->crawlingLegRoot[0];
            worldSpaceLegRoot[1] = bodyPos + ik->crawlingLegRoot[1];
        }

        switch (ik->state)
        {
            case PlayerState::Walking:
            {
                break;
            }
            case PlayerState::Crawling:
            {
                break;
            }
        }

        // Check if legs need to move
        unsigned char leg1Moved =
            glm::distance(ik->legIK[0].endpoint,
                          worldSpaceLegRoot[0]) > ik->legThreshold;
        unsigned char leg2Moved =
            glm::distance(ik->legIK[1].endpoint,
                          worldSpaceLegRoot[1]) > ik->legThreshold;

        b2ContactData contactData[10];
        int           shapeContactCount = b2Shape_GetContactData(
            ik->legCollider->shapeID, contactData, 10);
        int bodyContactCount = b2Body_GetContactData(
            ik->legCollider->body->bodyID, contactData, 10);

        unsigned char notNull = bodyContactCount != 0;

        static bool  leg1CanMove = true;
        static float legMoveTimer = 0.0f;
        static float blinkTimer = 0.0f;
        static float blinkHoldTimer = 0.2f;
        const float  LEG_MOVE_DELAY = 0.1f;

        legMoveTimer += engineState.deltaTime;
        blinkTimer += engineState.deltaTime;
        blinkHoldTimer -= engineState.deltaTime;

        // Handle leg movement with alternation
        if (legMoveTimer >= LEG_MOVE_DELAY)
        {
            if (leg1Moved && notNull && leg1CanMove)
            {
                ResetIK2D(ik->legIK[0], glm::vec2(multiplier, 0.0f));

                b2ShapeId polygonID =
                    b2Shape_GetFilter(contactData[0].shapeIdA)
                                .categoryBits == col_Static
                        ? contactData[0].shapeIdA
                        : contactData[0].shapeIdB;
                glm::vec2 ourLeg = worldSpaceLegRoot[0] +
                                   glm::vec2(0.1f * multiplier, 0.0f);
                b2Vec2 closestPoint = b2Shape_GetClosestPoint(
                    polygonID, b2Vec2(ourLeg.x, ourLeg.y));
                ik->legIK[0].endpoint =
                    glm::vec2(closestPoint.x, closestPoint.y);

                leg1CanMove = false;
                legMoveTimer = 0.0f;
            }
            else if (leg2Moved && notNull && !leg1CanMove)
            {
                ResetIK2D(ik->legIK[1], glm::vec2(multiplier, 0.0f));

                b2ShapeId polygonID =
                    b2Shape_GetFilter(contactData[0].shapeIdA)
                                .categoryBits == col_Static
                        ? contactData[0].shapeIdA
                        : contactData[0].shapeIdB;
                glm::vec2 ourLeg = worldSpaceLegRoot[1] +
                                   glm::vec2(0.1f * multiplier, 0.0f);
                b2Vec2 closestPoint = b2Shape_GetClosestPoint(
                    polygonID, b2Vec2(ourLeg.x, ourLeg.y));
                ik->legIK[1].endpoint =
                    glm::vec2(closestPoint.x, closestPoint.y);

                leg1CanMove = true;
                legMoveTimer = 0.0f;
            }
        }

        if (blinkTimer > 3.0f)
        {
            blinkTimer = 0.0f;
            blinkHoldTimer = 0.2f;
        }

        if (blinkHoldTimer >= 0)
        {
            ik->eyes->texture = ik->closedEyesTexture;
        }
        else
        {
            ik->eyes->texture = ik->eyesTexture;
        }

        ik->eyesTransform->position = ik->head->transform->position;

        if (notNull)
        {
            if (Input::GetKeyDown(Key::Z))
            {
                b2Body_SetLinearVelocity(
                    ik->legCollider->body->bodyID,
                    b2Vec2(0.0f, 0.0f));
                b2Body_SetLinearVelocity(ik->body[0]->bodyID,
                                         b2Vec2(0.0f, 0.0f));
                b2Body_SetLinearVelocity(ik->body[1]->bodyID,
                                         b2Vec2(0.0f, 0.0f));
                b2Body_ApplyForceToCenter(
                    ik->legCollider->body->bodyID,
                    b2Vec2(0.0f, ik->jumpSpeed), true);
            }

            if (Input::GetKeyDown(Key::Up))
            {
                ik->state = PlayerState::Walking;
            }
            else if (Input::GetKeyDown(Key::Down))
            {
                ik->state = PlayerState::Crawling;
            }
        }

        ik->legIK[0].points[0] = worldSpaceLegRoot[0];
        ik->legIK[1].points[0] = worldSpaceLegRoot[1];

        SolveIK2D(ik->legIK[0]);
        SolveIK2D(ik->legIK[1]);

        switch (ik->state)
        {
            case PlayerState::Walking:
            {
                ik->speed = ik->maxSpeed;
                ik->handIK[0].points[0] = ik->handRoot[0] + bodyPos;
                ik->handIK[1].points[0] = ik->handRoot[1] + bodyPos;
                break;
            }
            case PlayerState::Crawling:
            {
                ik->speed = ik->maxCrawlSpeed;
                ik->handIK[0].points[0] =
                    ik->crawlingHandRoot[0] + bodyPos;
                ik->handIK[1].points[0] =
                    ik->crawlingHandRoot[1] + bodyPos;
                break;
            }
        }

        glm::vec2 handPos =
            glm::vec2(ik->body[1]->transform->position);

        if (ik->handHold[0])
        {
            ResetIK2D(ik->handIK[0], glm::vec2(0.0f, -1.0f));
            ik->handIK[0].endpoint = handPos + glm::vec2(0.23f, 0.0f);
        }
        else
        {
            ResetIK2D(ik->handIK[1], glm::vec2(0.0f, -1.0f));
            ik->handIK[0].endpoint = handPos;
        }

        if (ik->handHold[1])
        {
            ik->handIK[1].endpoint =
                handPos + glm::vec2(-0.23f, 0.0f);
        }
        else
        {
            ik->handIK[1].endpoint = handPos;
        }

        SolveIK2D(ik->handIK[0]);
        SolveIK2D(ik->handIK[1]);

        if (Input::GetKey(Key::Left))
        {
            // Apply acceleration towards target speed
            if (ik->state != PlayerState::Crawling)
            {
                multiplier = -1.0f;
            }
            float targetSpeed = -ik->speed;
            float currentSpeed = b2Body_GetLinearVelocity(
                                     ik->legCollider->body->bodyID)
                                     .x;
            float acceleration =
                ik->acceleration * engineState.deltaTime;

            b2Body_SetLinearVelocity(
                ik->legCollider->body->bodyID,
                b2Vec2(MoveTowards(currentSpeed, targetSpeed,
                                   acceleration),
                       0.0f));
        }
        else if (Input::GetKey(Key::Right))
        {
            if (ik->state != PlayerState::Crawling)
            {
                multiplier = 1.0f;
            }
            float targetSpeed = ik->speed;
            float currentSpeed = b2Body_GetLinearVelocity(
                                     ik->legCollider->body->bodyID)
                                     .x;
            float acceleration =
                ik->acceleration * engineState.deltaTime;

            b2Body_SetLinearVelocity(
                ik->legCollider->body->bodyID,
                b2Vec2(MoveTowards(currentSpeed, targetSpeed,
                                   acceleration),
                       0.0f));
        }
        else
        {
            // Decelerate when no input
            float deceleration =
                ik->deceleration * engineState.deltaTime;

            float curVel = b2Body_GetLinearVelocity(
                               ik->legCollider->body->bodyID)
                               .x;

            b2Body_SetLinearVelocity(
                ik->legCollider->body->bodyID,
                b2Vec2(MoveTowards(curVel, 0.0f, deceleration),
                       0.0f));
        }

        if (Input::GetKeyDown(Key::LShift))
        {
            itemInReach = false;

            b2Circle    circle = {b2Vec2_zero, {0.05f}};
            b2Transform originTransform;
            glm::vec2   originPos = ik->body[0]->transform->position;
            originTransform.p = b2Vec2(originPos.x, originPos.y);
            originTransform.q = b2Rot_identity;

            b2QueryFilter filter = b2DefaultQueryFilter();
            filter.maskBits = col_Item;

            b2Vec2 translation = {10.0f, -5.0f};
            b2World_CastCircle(worldID, &circle, originTransform,
                               translation, filter, HandCastCallback,
                               &context);

            if (ik->handHold[0] && !ik->handHold[1])
            {
                ik->handHold[1] = true;
                ik->heldObjects[1] = (Collider2D*)b2Shape_GetUserData(
                    context.collidingShape);
                b2Filter filter = b2DefaultFilter();
                filter.categoryBits = col_Static;
                b2Shape_SetFilter(context.collidingShape, filter);
            }
            else if (ik->handHold[1] && !ik->handHold[0])
            {
                ik->handHold[0] = true;
                ik->heldObjects[0] = (Collider2D*)b2Shape_GetUserData(
                    context.collidingShape);
                b2Filter filter = b2DefaultFilter();
                filter.categoryBits = col_Static;
                b2Shape_SetFilter(context.collidingShape, filter);
            }
            else if (!ik->handHold[1] && !ik->handHold[0])
            {
                ik->handHold[0] = true;
                ik->heldObjects[0] = (Collider2D*)b2Shape_GetUserData(
                    context.collidingShape);
                b2Filter filter = b2DefaultFilter();
                filter.categoryBits = col_Static;
                b2Shape_SetFilter(context.collidingShape, filter);
            }
        }

        if (Input::GetKeyDown(Key::X))
        {
            if (ik->handHold[0])
            {
                b2Filter filter = b2DefaultFilter();
                filter.categoryBits = col_Item;
                b2Shape_SetFilter(ik->heldObjects[0]->shapeID,
                                  filter);

                b2Body_ApplyForceToCenter(
                    b2Shape_GetBody(ik->heldObjects[0]->shapeID),
                    b2Vec2(multiplier * 3.0f, 0.0f), true);
                ik->handHold[0] = false;
                ik->heldObjects[0] = nullptr;
            }
            else if (ik->handHold[1])
            {
                b2Filter filter = b2DefaultFilter();
                filter.categoryBits = col_Item;
                b2Shape_SetFilter(ik->heldObjects[0]->shapeID,
                                  filter);

                b2Body_ApplyForceToCenter(
                    b2Shape_GetBody(ik->heldObjects[0]->shapeID),
                    b2Vec2(multiplier * 3.0f, 0.0f), true);
                ik->handHold[1] = false;
                ik->heldObjects[1] = nullptr;
            }
        }

        if (ik->handHold[0])
        {
            b2Body_SetTransform(
                b2Shape_GetBody(ik->heldObjects[0]->shapeID),
                b2Vec2(ik->handIK[0].endpoint.x,
                       ik->handIK[0].endpoint.y),
                b2Rot_identity);
        }
        if (ik->handHold[1])
        {
            b2Body_SetTransform(
                b2Shape_GetBody(ik->heldObjects[1]->shapeID),
                b2Vec2(ik->handIK[1].endpoint.x,
                       ik->handIK[1].endpoint.y),
                b2Rot_identity);
        }

        Renderer::RenderLine2D(ik->legIK[0].points,
                               glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                               3.0f, 10.0f, false);
        Renderer::RenderLine2D(ik->legIK[1].points,
                               glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                               3.0f, 10.0f, false);
        Renderer::RenderLine2D(ik->handIK[0].points,
                               glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                               3.0f, 10.0f, false);
        Renderer::RenderLine2D(ik->handIK[1].points,
                               glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                               3.0f, 10.0f, false);
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

        ImGui::DragFloat2("HandRoot1",
                          glm::value_ptr(ik->handRoot[0]));
        ImGui::DragFloat2("HandRoot2",
                          glm::value_ptr(ik->handRoot[1]));
        ImGui::DragFloat("HandLength", &ik->handLength);

        ImGui::Checkbox("HandHold1", &ik->handHold[0]);
        ImGui::Checkbox("HandHold2", &ik->handHold[1]);

        ImGui::DragFloat2("CrawlingLegRoot1",
                          glm::value_ptr(ik->crawlingLegRoot[0]));
        ImGui::DragFloat2("CrawlingLegRoot2",
                          glm::value_ptr(ik->crawlingLegRoot[1]));

        ImGui::DragFloat2("CrawlingHandRoot1",
                          glm::value_ptr(ik->crawlingHandRoot[0]));
        ImGui::DragFloat2("CrawlingHandRoot2",
                          glm::value_ptr(ik->crawlingHandRoot[1]));

        ImGui::DragFloat("MaxCrawlSpeed", &ik->maxCrawlSpeed);
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
        {"MaxCrawlSpeed", ik->maxCrawlSpeed},
        {"Deceleration", ik->deceleration},
        {"HandRoot1", {ik->handRoot[0].x, ik->handRoot[0].y}},
        {"HandRoot2", {ik->handRoot[1].x, ik->handRoot[1].y}},
        {"CrawlingLegRoot1",
         {ik->crawlingLegRoot[0].x, ik->crawlingLegRoot[0].y}},
        {"CrawlingLegRoot2",
         {ik->crawlingLegRoot[1].x, ik->crawlingLegRoot[1].y}},
        {"CrawlingHandRoot1",
         {ik->crawlingHandRoot[0].x, ik->crawlingHandRoot[0].y}},
        {"CrawlingHandRoot2",
         {ik->crawlingHandRoot[1].x, ik->crawlingHandRoot[1].y}},
        {"HandLength", ik->handLength},
        {"JumpSpeed", ik->jumpSpeed}};

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
    if (j.contains("MaxCrawlSpeed"))
        ik->maxCrawlSpeed = j["MaxCrawlSpeed"];
    if (j.contains("HandRoot1"))
        ik->handRoot[0] = {j["HandRoot1"][0], j["HandRoot1"][1]};
    if (j.contains("HandRoot2"))
        ik->handRoot[1] = {j["HandRoot2"][0], j["HandRoot2"][1]};
    if (j.contains("HandLength"))
        ik->handLength = j["HandLength"];
    if (j.contains("CrawlingLegRoot1"))
        ik->crawlingLegRoot[0] = {j["CrawlingLegRoot1"][0],
                                  j["CrawlingLegRoot1"][1]};
    if (j.contains("CrawlingLegRoot2"))
        ik->crawlingLegRoot[1] = {j["CrawlingLegRoot2"][0],
                                  j["CrawlingLegRoot2"][1]};
    if (j.contains("CrawlingHandRoot1"))
        ik->crawlingHandRoot[0] = {j["CrawlingHandRoot1"][0],
                                   j["CrawlingHandRoot1"][1]};
    if (j.contains("CrawlingHandRoot2"))
        ik->crawlingHandRoot[1] = {j["CrawlingHandRoot2"][0],
                                   j["CrawlingHandRoot2"][1]};
    if (j.contains("JumpSpeed"))
        ik->jumpSpeed = j["JumpSpeed"];
}

REGISTER_COMPONENT(PlayerIK, PlayerIKDraw, PlayerIKSave,
                   PlayerIKLoad);
