#include "box2d/box2d.h"
#include <salmon/physics_2d.h>
#include <salmon/engine.h>
#include <imgui/imgui.h>
#include <glm/ext.hpp>
#include <salmon/editor.h>

void DebugDrawPolygon(const b2Vec2* vertices, int vertexCount,
                      b2HexColor color, void* context)
{
    std::vector<glm::vec2> points = {};

    for (int i = 0; i < vertexCount; i++)
    {
        points.push_back(glm::vec2(vertices[i].x, vertices[i].y));
    }

    Renderer::RenderLine2D(points, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
}

void DebugDrawSolidPolygon(b2Transform   transform,
                           const b2Vec2* vertices,
                           int32_t vertexCount, float radius,
                           b2HexColor color, void* context)
{
    std::vector<glm::vec2> points = {};

    for (int i = 0; i < vertexCount; i++)
    {
        points.push_back(glm::vec2(vertices[i].x, vertices[i].y));
    }

    Renderer::RenderLine2D(points, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
}

void DebugDrawCircle(b2Vec2 center, float radius, b2HexColor color,
                     void* context)
{
    glm::vec2              centroid = glm::vec2(center.x, center.y);
    std::vector<glm::vec2> points = {
        glm::vec2(centroid.x + radius, centroid.y + radius),
        glm::vec2(centroid.x - radius, centroid.y + radius),
        glm::vec2(centroid.x - radius, centroid.y - radius),
        glm::vec2(centroid.x + radius, centroid.y - radius)};

    Renderer::RenderLine2D(points, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
}

void DebugDrawSolidCircle(b2Transform transform, float radius,
                          b2HexColor color, void* context)
{
    glm::vec2 centroid = glm::vec2(transform.p.x, transform.p.y);
    std::vector<glm::vec2> points = {
        glm::vec2(centroid.x + radius, centroid.y + radius),
        glm::vec2(centroid.x - radius, centroid.y + radius),
        glm::vec2(centroid.x - radius, centroid.y - radius),
        glm::vec2(centroid.x + radius, centroid.y - radius)};

    Renderer::RenderLine2D(points, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
}

void DebugDrawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius,
                           b2HexColor color, void* context)
{
}

void DebugDrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color,
                      void* context)
{
    Renderer::RenderLine2D(
        {glm::vec2(p1.x, p1.y), glm::vec2(p2.x, p2.y)},
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
}

void DebugDrawTransform(b2Transform transform, void* context)
{
}

void DebugDrawPoint(b2Vec2 p, float size, b2HexColor color,
                    void* context)
{
}

void DebugDrawAABB(b2AABB aabb, b2HexColor color, void* context)
{
}

void InitPhysics2D()
{
    worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2(0.0f, -10.0f);

    worldID = b2CreateWorld(&worldDef);

    b2World_EnableSleeping(worldID, false);

    b2AABB bounds = {{-FLT_MAX, -FLT_MAX}, {FLT_MAX, FLT_MAX}};

    debugDraw = {};

    debugDraw.DrawPolygon = DebugDrawPolygon;
    debugDraw.DrawSolidPolygon = DebugDrawSolidPolygon;
    debugDraw.DrawCircle = DebugDrawCircle;
    debugDraw.DrawSolidCircle = DebugDrawSolidCircle;
    debugDraw.DrawSolidCapsule = DebugDrawSolidCapsule;
    debugDraw.DrawSegment = DebugDrawSegment;
    debugDraw.DrawTransform = DebugDrawTransform;
    debugDraw.DrawPoint = DebugDrawPoint;
    debugDraw.drawingBounds = bounds;

    debugDraw.useDrawingBounds = false;
    debugDraw.drawShapes = true;
    debugDraw.drawJoints = true;
    debugDraw.drawJointExtras = false;
    debugDraw.drawAABBs = false;
    debugDraw.drawMass = false;
    debugDraw.drawContacts = false;
    debugDraw.drawGraphColors = false;
    debugDraw.drawContactNormals = false;
    debugDraw.drawContactImpulses = false;
    debugDraw.drawFrictionImpulses = false;

    debugDraw.context = malloc(sizeof(char));
}

void Rigidbody2DFixCollidersStartSys()
{
    for (EntityID ent : SceneView<Rigidbody2D>(engineState.scene))
    {
        auto rigid = engineState.scene.Get<Rigidbody2D>(ent);
        auto col = engineState.scene.Get<Collider2D>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);

        if (col->body == nullptr)
        {
            col->body = rigid;
        }

        if (col->body->transform == nullptr)
        {
            col->body->transform = trans;
            rigid->transform = trans;
        }
    }
}

void Rigidbody2DStartSys()
{
    for (EntityID ent : SceneView<Rigidbody2D>(engineState.scene))
    {
        auto rigid = engineState.scene.Get<Rigidbody2D>(ent);

        rigid->bodyDef = b2DefaultBodyDef();

        rigid->bodyDef.position =
            b2Vec2(rigid->transform->position.x,
                   rigid->transform->position.y);

        if (rigid->type == rg2d_Dynamic)
        {
            rigid->bodyDef.type = b2_dynamicBody;
            rigid->bodyDef.angularDamping = rigid->angularDamping;
            rigid->bodyDef.linearDamping = rigid->linearDamping;
            rigid->bodyDef.fixedRotation = rigid->fixedRotation;
            rigid->bodyDef.enableSleep = !rigid->alwaysAwake;
            rigid->bodyDef.userData = (void*)rigid->userData;
        }

        rigid->bodyID = b2CreateBody(worldID, &rigid->bodyDef);
    }
}

void Rigidbody2DSys()
{
    for (EntityID ent : SceneView<Rigidbody2D>(engineState.scene))
    {
        auto rigid = engineState.scene.Get<Rigidbody2D>(ent);

        b2Vec2 pos = b2Body_GetPosition(rigid->bodyID);
        b2Rot  rot = b2Body_GetRotation(rigid->bodyID);

        rigid->transform->position = glm::vec3(pos.x, pos.y, 0.0f);
        rigid->transform->rotation.z = b2Rot_GetAngle(rot);
    }
}

void Collider2DStartSys()
{
    for (EntityID ent : SceneView<Collider2D>(engineState.scene))
    {
        auto col = engineState.scene.Get<Collider2D>(ent);
        auto rigid = col->body;

        switch (col->colliderType)
        {
            case rg2d_Box:
            {
                col->polygon =
                    b2MakeBox(col->halfwidths.x, col->halfwidths.y);
                col->shapeDef = b2DefaultShapeDef();

                if (col->body->type == rg2d_Dynamic)
                {
                    col->shapeDef.friction = rigid->friction;
                    col->shapeDef.density = rigid->mass;
                    col->shapeDef.restitution = rigid->restitution;
                }

                col->shapeID = b2CreatePolygonShape(
                    col->body->bodyID, &col->shapeDef, &col->polygon);
                break;
            }
            case rg2d_Circle:
            {
                col->shapeDef = b2DefaultShapeDef();

                if (col->body->type == rg2d_Dynamic)
                {
                    col->shapeDef.friction = rigid->friction;
                    col->shapeDef.density = rigid->mass;
                    col->shapeDef.restitution = rigid->restitution;
                }

                col->circle.radius = col->radius;
                col->circle.center =
                    b2Vec2(col->body->transform->position.x,
                           col->body->transform->position.y);

                col->shapeID = b2CreateCircleShape(
                    col->body->bodyID, &col->shapeDef, &col->circle);
                break;
            }
            case rg2d_Polygon:
            {
                b2Vec2 points[100] = {};
                glm::vec2 pos = col->body->transform->position;

                for (int i = 0; i < col->points.size(); ++i)
                {
                    points[i] =
                        b2Vec2(col->points[i].x, col->points[i].y);
                }

                b2Hull hull =
                    b2ComputeHull(points, col->points.size());
                float radius = 0.1f;
                col->polygon = b2MakePolygon(&hull, radius);

                if (hull.count < 3)
                {
                    std::cout << "Degenerate polygon hull was created\n";
                }

                col->shapeDef = b2DefaultShapeDef();

                if (col->body->type == rg2d_Dynamic)
                {
                    col->shapeDef.friction = rigid->friction;
                    col->shapeDef.density = rigid->mass;
                    col->shapeDef.restitution = rigid->restitution;
                }

                col->shapeID = b2CreatePolygonShape(
                    col->body->bodyID, &col->shapeDef, &col->polygon);

                if (!b2Shape_IsValid(col->shapeID))
                {
                    std::cout << "Polygn shape ain't valid yo!\n";
                }
                
                break;
            }
        }
    }
}

float timeStep = 1.0f / 60.0f;
int   subStepCount = 4;

void StepPhysics2D()
{
    b2World_Step(worldID, timeStep, subStepCount);
}

void DestroyPhysics2D()
{
    b2DestroyWorld(worldID);
}

REGISTER_START_SYSTEM(Rigidbody2DStartSys);
REGISTER_START_SYSTEM(Collider2DStartSys);
REGISTER_START_SYSTEM(Rigidbody2DFixCollidersStartSys);

REGISTER_SYSTEM(Rigidbody2DSys);

void Collider2DDebugSys()
{
    for (EntityID ent : SceneView<Collider2D>(engineState.scene))
    {
        auto col = engineState.scene.Get<Collider2D>(ent);
        auto rigid = engineState.scene.Get<Rigidbody2D>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);

        if (rigid == nullptr)
        {
            continue;
        }

        if (col->body == nullptr)
        {
            col->body = rigid;
        }

        if (col->body->transform == nullptr)
        {
            col->body->transform = trans;
        }

        if (col->colliderType == rg2d_Circle)
        {
            glm::vec2 topLeft =
                glm::vec2(col->body->transform->position) -
                glm::vec2(col->radius, col->radius);
            glm::vec2 topRight =
                glm::vec2(col->body->transform->position) +
                glm::vec2(col->radius, -col->radius);
            glm::vec2 bottomRight =
                glm::vec2(col->body->transform->position) +
                glm::vec2(-col->radius, col->radius);
            glm::vec2 bottomLeft =
                glm::vec2(col->body->transform->position) +
                glm::vec2(col->radius, col->radius);

            std::vector<glm::vec2> points = {topLeft, bottomRight,
                                             bottomLeft, topRight};

            Renderer::RenderLine2D(points,
                                   glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
        else if (col->colliderType == rg2d_Box)
        {
            // Get the rotation angle from transform
            float rotation =
                col->body->transform->rotation.z; // in radians
            float cosAngle = cos(rotation);
            float sinAngle = sin(rotation);

            // Calculate the four corners relative to center (before
            // rotation and translation)
            glm::vec2 relTopLeft(-col->halfwidths.x,
                                 col->halfwidths.y);
            glm::vec2 relTopRight(col->halfwidths.x,
                                  col->halfwidths.y);
            glm::vec2 relBottomRight(col->halfwidths.x,
                                     -col->halfwidths.y);
            glm::vec2 relBottomLeft(-col->halfwidths.x,
                                    -col->halfwidths.y);

            // Apply rotation to each corner
            glm::vec2 topLeft =
                glm::vec2(relTopLeft.x * cosAngle -
                              relTopLeft.y * sinAngle,
                          relTopLeft.x * sinAngle +
                              relTopLeft.y * cosAngle) +
                glm::vec2(col->body->transform->position);

            glm::vec2 topRight =
                glm::vec2(relTopRight.x * cosAngle -
                              relTopRight.y * sinAngle,
                          relTopRight.x * sinAngle +
                              relTopRight.y * cosAngle) +
                glm::vec2(col->body->transform->position);

            glm::vec2 bottomRight =
                glm::vec2(relBottomRight.x * cosAngle -
                              relBottomRight.y * sinAngle,
                          relBottomRight.x * sinAngle +
                              relBottomRight.y * cosAngle) +
                glm::vec2(col->body->transform->position);

            glm::vec2 bottomLeft =
                glm::vec2(relBottomLeft.x * cosAngle -
                              relBottomLeft.y * sinAngle,
                          relBottomLeft.x * sinAngle +
                              relBottomLeft.y * cosAngle) +
                glm::vec2(col->body->transform->position);

            std::vector<glm::vec2> points = {bottomLeft, topLeft,
                                             topRight, bottomRight};

            Renderer::RenderLine2D(points,
                                   glm::vec4(0.0f, 1.0f, 0.0f, 1.0));
        }
        else if (col->colliderType == rg2d_Polygon)
        {
            std::vector<glm::vec2> threedpoints;
            for (auto& point : col->points)
            {
                // Get the rotation angle from transform
                float rotation =
                    col->body->transform->rotation.z; // in radians

                // First create rotated point (before translation)
                float cosAngle = cos(rotation);
                float sinAngle = sin(rotation);

                glm::vec2 rotatedPoint(
                    point.x * cosAngle - point.y * sinAngle,
                    point.x * sinAngle + point.y * cosAngle);

                // Then add the position (translation)
                threedpoints.push_back(
                    rotatedPoint +
                    glm::vec2(col->body->transform->position));
            }

            Renderer::RenderLine2D(threedpoints,
                                   glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
    }
}

void Rigidbody2DDraw(Rigidbody2D* rb)
{
    if (ImGui::CollapsingHeader("Rigidbody2D"))
    {
        const char* types[] = {"Static", "Dynamic", "Kinematic"};
        int         currentType = static_cast<int>(rb->type);
        if (ImGui::Combo("rg2d Body Type", &currentType, types, 3))
        {
            rb->type = static_cast<Rigidbody2DType>(currentType);
        }

        ImGui::DragFloat("rg2d Mass", &rb->mass, 0.1f, 0.0f, 1000.0f);
        ImGui::DragFloat("rg2d Friction", &rb->friction, 0.1f, 0.0f,
                         1000.0f);
        ImGui::DragFloat("rg2d Linear Damping", &rb->linearDamping,
                         0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("rg2d Angular Damping", &rb->angularDamping,
                         0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("rg2d Restitution", &rb->restitution, 0.01f,
                         0.0f, 1.0f);
        ImGui::Checkbox("rg2d Fixed Rotation", &rb->fixedRotation);
        ImGui::Checkbox("rg2d Always Awake", &rb->alwaysAwake);
        ImGui::InputInt("rg2d UserData", &rb->userData);
    }
}

nlohmann::json Rigidbody2DSave(Rigidbody2D* rb)
{
    return {
        {"Mass", rb->mass},
        {"LinearDamping", rb->linearDamping},
        {"AngularDamping", rb->angularDamping},
        {"Restitution", rb->restitution},
        {"FixedRotation", rb->fixedRotation},
        {"UserData", rb->userData},
        {"AlwaysAwake", rb->alwaysAwake},
        {"Friction", rb->friction},
        {"Type", rb->type},
    };
}

void Rigidbody2DLoad(Rigidbody2D* rb, const nlohmann::json& j)
{
    rb->mass = j["Mass"];
    rb->linearDamping = j["LinearDamping"];
    rb->angularDamping = j["AngularDamping"];
    rb->restitution = j["Restitution"];
    rb->fixedRotation = j["FixedRotation"];
    rb->userData = j["UserData"];
    rb->alwaysAwake = j["AlwaysAwake"];
    rb->friction = j["Friction"];
    if (j.contains("Type"))
        rb->type = j["Type"];
}

float pdragThreshold = 0.3f;
int   pdragIndex = -1;

void Collider2DDraw(Collider2D* col)
{
    if (ImGui::CollapsingHeader("Collider2D"))
    {
        int colTypeValue = static_cast<int>(col->colliderType);
        if (ImGui::SliderInt("rg2d ColliderType2D", &colTypeValue, 0,
                             static_cast<int>(3)))
        {
            col->colliderType =
                static_cast<Collider2DType>(colTypeValue);
        }

        switch (col->colliderType)
        {
            case rg2d_Box:
                ImGui::DragFloat2("rg2d Half Widths",
                                  glm::value_ptr(col->halfwidths),
                                  0.1f, 0.0f);
                break;

            case rg2d_Circle:
                ImGui::DragFloat("rg2d Radius", &col->radius, 0.1f,
                                 0.0f);
                break;

            case rg2d_Polygon:
                ImGui::DragFloat("rg2d Threshold", &pdragThreshold);

                glm::vec2 mousePos =
                    engineState.camera->ScreenToWorld2D(
                        glm::vec2(Input::GetMouseInputHorizontal(),
                                  Input::GetMouseInputVertical()));

                bool mouse =
                    Input::GetMouseButton(MouseKey::LeftClick);
                bool hovering = false;

                for (size_t i = 0; i < col->points.size(); i++)
                {
                    std::string label = "Point " + std::to_string(i);
                    ImGui::DragFloat2(label.c_str(),
                                      glm::value_ptr(col->points[i]),
                                      0.1f);
                    float distance = glm::distance(
                        mousePos,
                        col->points[i] +
                            glm::vec2(
                                col->body->transform->position));
                    if (distance <= pdragThreshold && !mouse)
                    {
                        pdragIndex = i;
                        hovering = true;
                    }
                }

                if (!mouse && !hovering)
                {
                    pdragIndex = -1;
                }

                if (mouse && pdragIndex != -1)
                {
                    col->points[pdragIndex] =
                        mousePos -
                        glm::vec2(col->body->transform->position);
                }

                if (ImGui::Button("rg2d Add Point"))
                {
                    col->points.push_back(glm::vec2(0.0f));
                }
                break;
        }
    }
}

nlohmann::json Collider2DSave(Collider2D* col)
{
    nlohmann::json j = {
        {"Type", static_cast<int>(col->colliderType)}};

    switch (col->colliderType)
    {
        case rg2d_Box:
            j["HalfWidths"] = {col->halfwidths.x, col->halfwidths.y};
            break;

        case rg2d_Circle:
            j["Radius"] = col->radius;
            break;

        case rg2d_Polygon:
            std::vector<std::vector<float>> points;
            for (const auto& point : col->points)
            {
                points.push_back({point.x, point.y});
            }
            j["Points"] = points;
            break;
    }

    return j;
}

void Collider2DLoad(Collider2D* col, const nlohmann::json& j)
{
    col->colliderType = static_cast<Collider2DType>(j["Type"]);

    switch (col->colliderType)
    {
        case rg2d_Box:
            col->halfwidths = {j["HalfWidths"][0],
                               j["HalfWidths"][1]};
            break;

        case rg2d_Circle:
            col->circle.radius = j["Radius"];
            break;

        case rg2d_Polygon:
            col->points.clear();
            for (const auto& point : j["Points"])
            {
                col->points.push_back({point[0], point[1]});
            }
            break;
    }
}

REGISTER_COMPONENT(Rigidbody2D, Rigidbody2DDraw, Rigidbody2DSave,
                   Rigidbody2DLoad);
REGISTER_COMPONENT(Collider2D, Collider2DDraw, Collider2DSave,
                   Collider2DLoad);
