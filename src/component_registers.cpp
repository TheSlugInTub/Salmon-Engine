#include <sm2d/functions.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <salmon/editor.h>
#include <salmon/components.h>
#include <sm2d/types.h>
#include <salmon/particle_system.h>
#include <salmon/sprite_animation.h>

// -------------------

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
    return {{"Position", {trans->position.x, trans->position.y, trans->position.z}},
            {"Rotation", {trans->rotation.x, trans->rotation.y, trans->rotation.z}},
            {"Scale", {trans->scale.x, trans->scale.y, trans->scale.z}}};
}

void TransformLoad(Transform* trans, const nlohmann::json& j)
{
    trans->position = {j["Position"][0], j["Position"][1], j["Position"][2]};
    trans->rotation = {j["Rotation"][0], j["Rotation"][1], j["Rotation"][2]};
    trans->scale = {j["Scale"][0], j["Scale"][1], j["Scale"][2]};
}

// -------------------

void NameDraw(Name* name)
{
}

nlohmann::json NameSave(Name* name)
{
    return {{"EntName", name->name}};
}

void NameLoad(Name* name, const nlohmann::json& j)
{
    name->name = j["EntName"];
}

// -------------------

void SpriteRendererDraw(SpriteRenderer* sprite)
{
    if (ImGui::CollapsingHeader("SpriteRenderer"))
    {
        char texBuffer[128];
        std::strncpy(texBuffer, sprite->texturePath.c_str(), sizeof(texBuffer));
        if (ImGui::InputText("TexturePath", texBuffer, sizeof(texBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            sprite->texturePath = std::string(texBuffer);
            sprite->texture = Utils::LoadTexture(texBuffer);
        }

        ImGui::DragFloat4("Color", glm::value_ptr(sprite->color));
        ImGui::Checkbox("Billboarded", &sprite->billboard);
    }
}

nlohmann::json SpriteRendererSave(SpriteRenderer* sprite)
{
    return {{"TexturePath", sprite->texturePath},
            {"Color", {sprite->color.x, sprite->color.y, sprite->color.z, sprite->color.w}},
            {"Billboard", sprite->billboard}};
}

void SpriteRendererLoad(SpriteRenderer* sprite, const nlohmann::json& j)
{
    sprite->texturePath = j["TexturePath"];
    sprite->billboard = j["Billboard"];
    sprite->color = {j["Color"][0], j["Color"][1], j["Color"][2], j["Color"][3]};
    sprite->texture = Utils::LoadTexture(sprite->texturePath.c_str());
}

// -------------------

void MeshRendererDraw(MeshRenderer* mesh)
{
    if (ImGui::CollapsingHeader("MeshRenderer"))
    {
        char texBuffer[128];
        std::strncpy(texBuffer, mesh->texturePath.c_str(), sizeof(texBuffer));
        if (ImGui::InputText("TexturePath", texBuffer, sizeof(texBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            mesh->texturePath = std::string(texBuffer);
            mesh->texture = Utils::LoadTexture(texBuffer);
        }

        char modelBuffer[128];
        std::strncpy(modelBuffer, mesh->modelPath.c_str(), sizeof(modelBuffer));
        if (ImGui::InputText("ModelPath", modelBuffer, sizeof(modelBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            mesh->modelPath = std::string(modelBuffer);
            mesh->model =
                Model(mesh->modelPath, mesh->model.gammaCorrection, mesh->model.extractTexture);
        }
        ImGui::Checkbox("GammaCorrection", &mesh->model.gammaCorrection);
        ImGui::Checkbox("ExtractTexture", &mesh->model.extractTexture);
        ImGui::DragFloat4("ModelColor", glm::value_ptr(mesh->color));
    }
}

nlohmann::json MeshRendererSave(MeshRenderer* mesh)
{
    return {
        {"ModelPath", mesh->modelPath},
        {"TexturePath", mesh->texturePath},
        {"Color", {mesh->color.x, mesh->color.y, mesh->color.z, mesh->color.w}},
    };
}

void MeshRendererLoad(MeshRenderer* mesh, const nlohmann::json& j)
{
    mesh->modelPath = j["ModelPath"];
    mesh->texturePath = j["TexturePath"];
    mesh->color = {j["Color"][0], j["Color"][1], j["Color"][2], j["Color"][3]};
}

// -------------------

void RigidBody3DDraw(RigidBody3D* rigid)
{
    const char* types[] = {"Box", "Sphere", "Capsule", "Mesh"};
    int         currentType = static_cast<int>(rigid->colliderType);
    if (ImGui::Combo("Collider Type", &currentType, types, 3))
    {
        rigid->colliderType = static_cast<ColliderType>(currentType);
    }

    const char* states[] = {"Dynamic", "Static"};
    int         currentState = static_cast<int>(rigid->state);
    if (ImGui::Combo("Collider Type", &currentState, states, 3))
    {
        rigid->state = static_cast<BodyState>(currentState);
    }

    ImGui::DragFloat3("BoxSize", glm::value_ptr(rigid->boxSize));
    ImGui::DragFloat("SphereRadius", &rigid->sphereRadius);
    ImGui::DragFloat("CapsuleRadius", &rigid->capsuleRadius);
    ImGui::DragFloat("CapsuleHeight", &rigid->capsuleHeight);
    ImGui::DragFloat3("Offset", glm::value_ptr(rigid->offset));
    ImGui::InputInt("GroupID", &rigid->groupID);
}

nlohmann::json RigidBody3DSave(RigidBody3D* rigid)
{
    return {{"ColliderType", rigid->colliderType},
            {"BodyState", rigid->state},
            {"BoxSize", {rigid->boxSize.x, rigid->boxSize.y, rigid->boxSize.z}},
            {"SphereRadius", rigid->sphereRadius},
            {"CapsuleRadius", rigid->capsuleRadius},
            {"CapsuleHeight", rigid->capsuleHeight},
            {"Offset", {rigid->offset.x, rigid->offset.y, rigid->offset.z}},
            {"GroupID", rigid->groupID}};
}

void RigidBody3DLoad(RigidBody3D* rigid, const nlohmann::json& j)
{
    rigid->colliderType = j["ColliderType"];
    rigid->state = j["BodyState"];
    rigid->boxSize = {j["BoxSize"][0], j["BoxSize"][1], j["BosSize"][2]};
    rigid->capsuleRadius = j["CapsuleRadius"];
    rigid->sphereRadius = j["SphereRadius"];
    rigid->capsuleHeight = j["CapsuleHeight"];
    rigid->offset = {j["Offset"][0], j["Offset"][1], j["Offset"][2]};
    rigid->groupID = j["GroupID"];
}

// -------------------

void AnimatorDraw(Animator* anim)
{
    if (ImGui::CollapsingHeader("Animator"))
    {
        ImGui::Checkbox("Playing", &anim->playing);
        ImGui::Checkbox("Looping", &anim->looping);
        ImGui::DragFloat("Speed", &anim->speed, 0.1f, 0.0f, 10.0f);
        ImGui::DragFloat("Current Time", &anim->currentTime);

        char animBuffer[128];
        std::strncpy(animBuffer, anim->animationPath.c_str(), sizeof(animBuffer));
        if (ImGui::InputText("AnimationPath", animBuffer, sizeof(animBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (anim->currentAnimation == nullptr)
                delete anim->currentAnimation;
            anim->animationPath = std::string(animBuffer);
            anim->currentAnimation = new Animation(anim->animationPath, anim->model);
        }
        ImGui::Text("If you want to play multiple animations, please make Animation objects in "
                    "systems and assign them to the currentAnimation of the Animator.");
    }
}

nlohmann::json AnimatorSave(Animator* anim)
{
    return {{"Playing", anim->playing},
            {"Looping", anim->looping},
            {"Speed", anim->speed},
            {"CurrentTime", anim->currentTime},
            {"AnimationPath", anim->animationPath}};
}

void AnimatorLoad(Animator* anim, const nlohmann::json& j)
{
    anim->playing = j["Playing"];
    anim->looping = j["Looping"];
    anim->speed = j["Speed"];
    anim->currentTime = j["CurrentTime"];
    anim->animationPath = j["AnimationPath"];
}

// -------------------

void ParticleSystemDraw(ParticleSystem* ps)
{
    if (ImGui::CollapsingHeader("Particle System"))
    {
        char texBuffer[128];
        std::strncpy(texBuffer, ps->texturePath.c_str(), sizeof(texBuffer));
        if (ImGui::InputText("ParticleTexturePath", texBuffer, sizeof(texBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            ps->texturePath = std::string(texBuffer);
            ps->texture = Utils::LoadTexture(texBuffer);
        }

        ImGui::Checkbox("Playing", &ps->playing);
        ImGui::Checkbox("Looping", &ps->looping);
        ImGui::Checkbox("Billboarded", &ps->billboarded);

        ImGui::DragFloat3("Starting Position", glm::value_ptr(ps->startingPosition));
        ImGui::DragFloat3("Starting Rotation", glm::value_ptr(ps->startingRotation));
        ImGui::DragFloat3("Starting Size", glm::value_ptr(ps->startingSize));

        ImGui::ColorEdit4("Starting Color", glm::value_ptr(ps->startingColor));
        ImGui::ColorEdit4("Color Over Time", glm::value_ptr(ps->colorOverTime));

        ImGui::DragFloat("Size Over Time", &ps->sizeOverTime, 0.1f);
        ImGui::DragFloat3("Rotation Over Time", glm::value_ptr(ps->rotationOverTime));

        ImGui::DragFloat3("Force", glm::value_ptr(ps->force));
        ImGui::DragFloat("Force Magnitude", &ps->forceMagnitude, 0.1f);
        ImGui::DragFloat3("Force Randomness", glm::value_ptr(ps->forceRandomness));
        ImGui::DragFloat("Force Over Time", &ps->forceOverTime, 0.1f);

        ImGui::DragFloat3("Gravity", glm::value_ptr(ps->gravity));

        ImGui::DragFloat("Particle Lifetime", &ps->particleLifetime, 0.1f);
        ImGui::DragFloat("Duration", &ps->duration, 0.1f);
        ImGui::DragFloat("Particle Rate", &ps->particleRate, 0.1f);
        ImGui::InputInt("Max Particles", (int*)&ps->maxParticles);
    }
}

nlohmann::json ParticleSystemSave(ParticleSystem* ps)
{
    return {
        {"Playing", ps->playing},
        {"Looping", ps->looping},
        {"Billboarded", ps->billboarded},
        {"StartingPosition",
         {ps->startingPosition.x, ps->startingPosition.y, ps->startingPosition.z}},
        {"StartingRotation",
         {ps->startingRotation.x, ps->startingRotation.y, ps->startingRotation.z}},
        {"StartingSize", {ps->startingSize.x, ps->startingSize.y, ps->startingSize.z}},
        {"StartingColor",
         {ps->startingColor.r, ps->startingColor.g, ps->startingColor.b, ps->startingColor.a}},
        {"ColorOverTime",
         {ps->colorOverTime.r, ps->colorOverTime.g, ps->colorOverTime.b, ps->colorOverTime.a}},
        {"SizeOverTime", ps->sizeOverTime},
        {"RotationOverTime",
         {ps->rotationOverTime.x, ps->rotationOverTime.y, ps->rotationOverTime.z}},
        {"Force", {ps->force.x, ps->force.y, ps->force.z}},
        {"ForceMagnitude", ps->forceMagnitude},
        {"ForceRandomness", {ps->forceRandomness.x, ps->forceRandomness.y, ps->forceRandomness.z}},
        {"ForceOverTime", ps->forceOverTime},
        {"Gravity", {ps->gravity.x, ps->gravity.y, ps->gravity.z}},
        {"ParticleLifetime", ps->particleLifetime},
        {"Duration", ps->duration},
        {"ParticleRate", ps->particleRate},
        {"TexturePath", ps->texturePath},
        {"MaxParticles", ps->maxParticles}};
}

void ParticleSystemLoad(ParticleSystem* ps, const nlohmann::json& j)
{
    ps->playing = j["Playing"];
    ps->looping = j["Looping"];
    ps->billboarded = j["Billboarded"];
    ps->startingPosition = {j["StartingPosition"][0], j["StartingPosition"][1],
                            j["StartingPosition"][2]};
    ps->startingRotation = {j["StartingRotation"][0], j["StartingRotation"][1],
                            j["StartingRotation"][2]};
    ps->startingSize = {j["StartingSize"][0], j["StartingSize"][1], j["StartingSize"][2]};
    ps->startingColor = {j["StartingColor"][0], j["StartingColor"][1], j["StartingColor"][2],
                         j["StartingColor"][3]};
    ps->colorOverTime = {j["ColorOverTime"][0], j["ColorOverTime"][1], j["ColorOverTime"][2],
                         j["ColorOverTime"][3]};
    ps->sizeOverTime = j["SizeOverTime"];
    ps->rotationOverTime = {j["RotationOverTime"][0], j["RotationOverTime"][1],
                            j["RotationOverTime"][2]};
    ps->force = {j["Force"][0], j["Force"][1], j["Force"][2]};
    ps->forceMagnitude = j["ForceMagnitude"];
    ps->forceRandomness = {j["ForceRandomness"][0], j["ForceRandomness"][1],
                           j["ForceRandomness"][2]};
    ps->forceOverTime = j["ForceOverTime"];
    ps->gravity = {j["Gravity"][0], j["Gravity"][1], j["Gravity"][2]};
    ps->particleLifetime = j["ParticleLifetime"];
    ps->duration = j["Duration"];
    ps->particleRate = j["ParticleRate"];
    ps->maxParticles = j["MaxParticles"];
    ps->texturePath = j["TexturePath"];
    ps->texture = Utils::LoadTexture(ps->texturePath.c_str());
}

// -------------------

void LightDraw(Light* light)
{
    if (ImGui::CollapsingHeader("Light"))
    {
        ImGui::DragFloat3("Light Position", glm::value_ptr(light->position));
        ImGui::ColorEdit4("Light Color", glm::value_ptr(light->color));
        ImGui::DragFloat("Radius", &light->radius, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Inner Radius", &light->innerRadius, 0.1f, 0.0f, light->radius);
        ImGui::DragFloat("Intensity", &light->intensity, 0.1f, 0.0f, 10.0f);
        ImGui::Checkbox("Cast Shadows", &light->castShadows);
    }
}

nlohmann::json LightSave(Light* light)
{
    return {{"Position", {light->position.x, light->position.y, light->position.z}},
            {"Color", {light->color.r, light->color.g, light->color.b, light->color.a}},
            {"Radius", light->radius},
            {"InnerRadius", light->innerRadius},
            {"Intensity", light->intensity},
            {"CastShadows", light->castShadows}};
}

void LightLoad(Light* light, const nlohmann::json& j)
{
    light->position = {j["Position"][0], j["Position"][1], j["Position"][2]};
    light->color = {j["Color"][0], j["Color"][1], j["Color"][2], j["Color"][3]};
    light->radius = j["Radius"];
    light->innerRadius = j["InnerRadius"];
    light->intensity = j["Intensity"];
    light->castShadows = j["CastShadows"];
}

// -------------------

void RigidbodyDraw(sm2d::Rigidbody* rb)
{
    if (ImGui::CollapsingHeader("Rigidbody"))
    {
        const char* types[] = {"Static", "Dynamic", "Kinematic"};
        int         currentType = static_cast<int>(rb->type);
        if (ImGui::Combo("Body Type", &currentType, types, 3))
        {
            rb->type = static_cast<sm2d::BodyType>(currentType);
        }

        ImGui::DragFloat("Mass", &rb->mass, 0.1f, 0.0f, 1000.0f);
        ImGui::DragFloat("Linear Damping", &rb->linearDamping, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Angular Damping", &rb->angularDamping, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Restitution", &rb->restitution, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Moment of Inertia", &rb->momentOfInertia, 0.1f, 0.0f, 1000.0f);
        ImGui::Checkbox("Fixed Rotation", &rb->fixedRotation);
        ImGui::Checkbox("Awake", &rb->awake);
        ImGui::DragFloat2("Linear Velocity", glm::value_ptr(rb->linearVelocity), 0.1f);
        ImGui::DragFloat("Angular Velocity", &rb->angularVelocity, 0.1f);
        ImGui::DragFloat2("Force", glm::value_ptr(rb->force), 0.1f);
        ImGui::DragFloat("Torque", &rb->torque, 0.1f);
    }
}

nlohmann::json RigidbodySave(sm2d::Rigidbody* rb)
{
    return {{"Type", static_cast<int>(rb->type)},
            {"Mass", rb->mass},
            {"LinearDamping", rb->linearDamping},
            {"AngularDamping", rb->angularDamping},
            {"Restitution", rb->restitution},
            {"FixedRotation", rb->fixedRotation},
            {"MomentOfInertia", rb->momentOfInertia},
            {"Awake", rb->awake},
            {"LinearVelocity", {rb->linearVelocity.x, rb->linearVelocity.y}},
            {"AngularVelocity", rb->angularVelocity},
            {"Force", {rb->force.x, rb->force.y}},
            {"Torque", rb->torque}};
}

void RigidbodyLoad(sm2d::Rigidbody* rb, const nlohmann::json& j)
{
    rb->type = static_cast<sm2d::BodyType>(j["Type"]);
    rb->mass = j["Mass"];
    rb->linearDamping = j["LinearDamping"];
    rb->angularDamping = j["AngularDamping"];
    rb->restitution = j["Restitution"];
    rb->fixedRotation = j["FixedRotation"];
    rb->momentOfInertia = j["MomentOfInertia"];
    rb->awake = j["Awake"];
    rb->linearVelocity = {j["LinearVelocity"][0], j["LinearVelocity"][1]};
    rb->angularVelocity = j["AngularVelocity"];
    rb->force = {j["Force"][0], j["Force"][1]};
    rb->torque = j["Torque"];
}

// -------------------

void ColliderDraw(sm2d::Collider* col)
{
    if (ImGui::CollapsingHeader("Collider"))
    {
        const char* types[] = {"AABB", "Circle", "Polygon"};
        int         currentType = static_cast<int>(col->type);
        if (ImGui::Combo("Collider Type", &currentType, types, 3))
        {
            col->type = static_cast<sm2d::ColliderType>(currentType);
        }

        switch (col->type)
        {
            case sm2d::sm2d_AABB:
                ImGui::DragFloat2("Half Widths", glm::value_ptr(col->aabb.halfwidths), 0.1f, 0.0f);
                break;

            case sm2d::sm2d_Circle:
                ImGui::DragFloat("Radius", &col->circle.radius, 0.1f, 0.0f);
                break;

            case sm2d::sm2d_Polygon:
                ImGui::DragFloat2("Center", glm::value_ptr(col->polygon.center), 0.1f);

                for (size_t i = 0; i < col->polygon.points.size(); i++)
                {
                    std::string label = "Point " + std::to_string(i);
                    ImGui::DragFloat2(label.c_str(), glm::value_ptr(col->polygon.points[i]), 0.1f);
                }

                if (ImGui::Button("Add Point"))
                {
                    col->polygon.points.push_back(glm::vec2(0.0f));
                }
                break;
        }

        if (ImGui::Button("Update collider"))
        {
            sm2d::UpdateCollider(col);
        }
    }
}

nlohmann::json ColliderSave(sm2d::Collider* col)
{
    nlohmann::json j = {{"Type", static_cast<int>(col->type)}};

    switch (col->type)
    {
        case sm2d::sm2d_AABB:
            j["HalfWidths"] = {col->aabb.halfwidths.x, col->aabb.halfwidths.y};
            break;

        case sm2d::sm2d_Circle:
            j["Radius"] = col->circle.radius;
            break;

        case sm2d::sm2d_Polygon:
            j["Center"] = {col->polygon.center.x, col->polygon.center.y};
            std::vector<std::vector<float>> points;
            for (const auto& point : col->polygon.points) { points.push_back({point.x, point.y}); }
            j["Points"] = points;
            break;
    }

    return j;
}

void ColliderLoad(sm2d::Collider* col, const nlohmann::json& j)
{
    col->type = static_cast<sm2d::ColliderType>(j["Type"]);

    switch (col->type)
    {
        case sm2d::sm2d_AABB:
            col->aabb.halfwidths = {j["HalfWidths"][0], j["HalfWidths"][1]};
            break;

        case sm2d::sm2d_Circle:
            col->circle.radius = j["Radius"];
            break;

        case sm2d::sm2d_Polygon:
            col->polygon.center = {j["Center"][0], j["Center"][1]};
            col->polygon.points.clear();
            for (const auto& point : j["Points"])
            {
                col->polygon.points.push_back({point[0], point[1]});
            }
            break;
    }
}

// -------------------

void SpriteAnimatorDraw(SpriteAnimator* sprite)
{
    if (ImGui::CollapsingHeader("SpriteAnimator"))
    {
        ImGui::Text("Please add animations to the sprite animator through systems.");
    }
}

nlohmann::json SpriteAnimatorSave(SpriteAnimator* sprite)
{
    return nlohmann::json {};
}

void SpriteAnimatorLoad(SpriteAnimator* sprite, nlohmann::json j)
{
}

// -------------------

using namespace sm2d;
REGISTER_COMPONENT(Name, NameDraw, NameSave, NameLoad);
REGISTER_COMPONENT(Transform, TransformDraw, TransformSave, TransformLoad);
REGISTER_COMPONENT(SpriteRenderer, SpriteRendererDraw, SpriteRendererSave, SpriteRendererLoad);
REGISTER_COMPONENT(MeshRenderer, MeshRendererDraw, MeshRendererSave, MeshRendererLoad);
REGISTER_COMPONENT(Animator, AnimatorDraw, AnimatorSave, AnimatorLoad);
REGISTER_COMPONENT(ParticleSystem, ParticleSystemDraw, ParticleSystemSave, ParticleSystemLoad);
REGISTER_COMPONENT(Rigidbody, RigidbodyDraw, RigidbodySave, RigidbodyLoad);
REGISTER_COMPONENT(Light, LightDraw, LightSave, LightLoad);
REGISTER_COMPONENT(Collider, ColliderDraw, ColliderSave, ColliderLoad);
REGISTER_COMPONENT(RigidBody3D, RigidBody3DDraw, RigidBody3DSave, RigidBody3DLoad);
