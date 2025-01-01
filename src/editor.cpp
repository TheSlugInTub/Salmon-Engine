#include <salmon/utils.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <salmon/editor.h>
#include <salmon/components.h>
#include <sm2d/colliders.h>
#include <sm2d/functions.h>

void DrawHierarchy()
{
    ImGui::Begin("Hierarchy");
    for (size_t i = 0; i < engineState.scene.entities.size(); ++i)
    {
        ImGui::PushID(static_cast<int>(i));

        auto name = engineState.scene.Get<Name>(engineState.scene.entities[i].id);
        bool selected = selectedEntityIndex == static_cast<int>(i);
        if (ImGui::Selectable(name->name.c_str(), selected))
        {
            selectedEntityIndex = static_cast<int>(i);
            selectedEntity = engineState.scene.entities[selectedEntityIndex].id;
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload("DND_DEMO_CELL", &i, sizeof(size_t));
            ImGui::Text("Dragging Object %d", i);
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
            {
                IM_ASSERT(payload->DataSize == sizeof(size_t));
                size_t payload_n = *(const size_t*)payload->Data;
                if (payload_n != i)
                {
                    std::swap(engineState.scene.entities[payload_n], engineState.scene.entities[i]);
                    if (selectedEntityIndex == static_cast<int>(i))
                    {
                        selectedEntityIndex = static_cast<int>(payload_n);
                        selectedEntity = engineState.scene.entities[selectedEntityIndex].id;
                    }
                    else if (selectedEntityIndex == static_cast<int>(payload_n))
                    {
                        selectedEntityIndex = static_cast<int>(i);
                        selectedEntity = engineState.scene.entities[selectedEntityIndex].id;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::PopID();
    }

    if (ImGui::BeginPopupContextWindow())
    {
        if (ImGui::MenuItem("Add Entity"))
        {
            EntityID ent = engineState.scene.AddEntity();
            engineState.scene.Assign<Transform>(ent);
            engineState.scene.AssignParam<Name>(ent, "Entity");
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void DrawInspector()
{
    if (selectedEntityIndex == -1)
        return;

    ImGui::Begin("Inspector");

    auto name = engineState.scene.Get<Name>(selectedEntity);
    char nameBuffer[128];
    std::strncpy(nameBuffer, name->name.c_str(), sizeof(nameBuffer));
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
    {
        name->name = std::string(nameBuffer);
    }

    ComponentRegistry::Instance().DrawAll();

    ImGui::End();
}

void SaveScene()
{
    ;
    ;
}

void LoadScene()
{
    ;
    ;
}

void DrawTray()
{
    if (ImGui::Button("Save")) {}
}

// void DrawInspector()
// {
//     if (selectedEntityIndex == -1)
//         return;
//
//     ImGui::Begin("Inspector");
//
//     // ----------------------------------
//     auto name = engineState.scene.Get<Name>(selectedEntity);
//     char nameBuffer[128];
//     std::strncpy(nameBuffer, name->name.c_str(), sizeof(nameBuffer));
//     if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
//     {
//         name->name = std::string(nameBuffer);
//     }
//     // ----------------------------------
//     auto trans = engineState.scene.Get<Transform>(selectedEntity);
//     if (trans != nullptr)
//     {
//         if (ImGui::CollapsingHeader("Transform"))
//         {
//             ImGui::DragFloat3("Position", glm::value_ptr(trans->position));
//             ImGui::DragFloat3("Rotation", glm::value_ptr(trans->rotation));
//             ImGui::DragFloat3("Scale", glm::value_ptr(trans->scale));
//         }
//     }
//     // ----------------------------------
//     auto sprite = engineState.scene.Get<SpriteRenderer>(selectedEntity);
//     if (sprite != nullptr)
//     {
//         if (ImGui::CollapsingHeader("SpriteRenderer"))
//         {
//             char texBuffer[128];
//             std::strncpy(texBuffer, sprite->texturePath.c_str(), sizeof(texBuffer));
//             if (ImGui::InputText("TexturePath", texBuffer, sizeof(texBuffer)))
//             {
//                 sprite->texturePath = std::string(texBuffer);
//                 sprite->texture = Utils::LoadTexture(sprite->texturePath.c_str());
//             }
//             ImGui::DragFloat4("Color", glm::value_ptr(sprite->color));
//             ImGui::Checkbox("Billboard", &sprite->billboard);
//         }
//     }
//     // ----------------------------------
//     auto mesh = engineState.scene.Get<MeshRenderer>(selectedEntity);
//     if (mesh != nullptr)
//     {
//         if (ImGui::CollapsingHeader("MeshRenderer"))
//         {
//             ImGui::Checkbox("Gamma correction", &mesh->model.gammaCorrection);
//             ImGui::Checkbox("Texture from model", &mesh->model.extractTexture);
//             char meshBuffer[128];
//             std::strncpy(meshBuffer, mesh->modelPath.c_str(), sizeof(meshBuffer));
//             if (ImGui::InputText("ModelPath", meshBuffer, sizeof(meshBuffer)))
//             {
//                 mesh->modelPath = std::string(meshBuffer);
//                 mesh->model =
//                     Model(mesh->modelPath, mesh->model.gammaCorrection,
//                     mesh->model.extractTexture);
//             }
//             ImGui::DragFloat4("Model color", glm::value_ptr(mesh->color));
//             char texBuffer[128];
//             std::strncpy(texBuffer, mesh->texturePath.c_str(), sizeof(texBuffer));
//             if (ImGui::InputText("TexturePath", texBuffer, sizeof(texBuffer)))
//             {
//                 mesh->texturePath = std::string(texBuffer);
//                 mesh->texture = Utils::LoadTexture(mesh->texturePath.c_str());
//             }
//         }
//     }
//     // ----------------------------------
//     auto rigid = engineState.scene.Get<RigidBody3D>(selectedEntity);
//     if (rigid != nullptr)
//     {
//         if (ImGui::CollapsingHeader("RigidBody3D"))
//         {
//             ImGui::InputInt("ColliderType", &rigid->colliderType);
//             ImGui::InputInt("MovementType", &rigid->state);
//             ImGui::DragFloat3("BoxSize", glm::value_ptr(rigid->boxSize));
//             ImGui::DragFloat("SphereRadius", &rigid->sphereRadius);
//             ImGui::DragFloat("CapsuleRadius", &rigid->capsuleRadius);
//             ImGui::DragFloat("CapsuleHeight", &rigid->capsuleHeight);
//             ImGui::DragFloat3("Offset", glm::value_ptr(rigid->offset));
//             ImGui::InputInt("GroupID", &rigid->groupID);
//         }
//     }
//     // ----------------------------------
//     auto anim = engineState.scene.Get<Animator>(selectedEntity);
//     if (anim != nullptr && mesh != nullptr)
//     {
//         if (ImGui::CollapsingHeader("Animator3D"))
//         {
//             ImGui::Checkbox("Playing", &anim->playing);
//             char animBuffer[128];
//             std::strncpy(animBuffer, anim->animationPath.c_str(), sizeof(animBuffer));
//             if (ImGui::InputText("AnimationPath", animBuffer, sizeof(animBuffer)))
//             {
//                 if (anim->currentAnimation != nullptr)
//                 {
//                     delete anim->currentAnimation;
//                 }
//                 anim->animationPath = std::string(animBuffer);
//                 anim->currentAnimation = new Animation(anim->animationPath, &mesh->model);
//             }
//             ImGui::Checkbox("Looping", &anim->looping);
//             ImGui::DragFloat("Speed", &anim->speed);
//         }
//     }
//     // ----------------------------------
//     auto rigid2 = engineState.scene.Get<sm2d::Rigidbody>(selectedEntity);
//     if (rigid2 != nullptr)
//     {
//         if (ImGui::CollapsingHeader("RigidBody2D"))
//         {
//             ImGui::InputInt("MoveType", &rigid2->type);
//             ImGui::DragFloat("Mass", &rigid2->mass);
//             ImGui::Checkbox("Awake", &rigid2->awake);
//             ImGui::Checkbox("HasMoved", &rigid2->hasMoved);
//             ImGui::DragFloat("LinearDamping", &rigid2->linearDamping);
//             ImGui::DragFloat("AngularDamping", &rigid2->angularDamping);
//             ImGui::DragFloat("Restitution", &rigid2->restitution);
//             ImGui::Checkbox("FixedRotation", &rigid2->fixedRotation);
//             ImGui::DragFloat("Inertia", &rigid2->momentOfInertia);
//         }
//     }
//     // ----------------------------------
//     auto light = engineState.scene.Get<Light>(selectedEntity);
//     if (light != nullptr)
//     {
//         if (ImGui::CollapsingHeader("Light3D"))
//         {
//             ImGui::DragFloat3("LightPos", glm::value_ptr(light->position));
//             ImGui::DragFloat4("LightColor", glm::value_ptr(light->color));
//             ImGui::DragFloat("LightRadius", &light->radius);
//             ImGui::DragFloat("LightIntensity", &light->intensity);
//             ImGui::Checkbox("CastsShadows", &light->castShadows);
//         }
//     }
//     // ----------------------------------
//     auto col = engineState.scene.Get<sm2d::Collider>(selectedEntity);
//     if (col != nullptr)
//     {
//         if (ImGui::CollapsingHeader("Collider2D"))
//         {
//             bool colliderEdited = false;
//
//             ImGui::InputInt("ColliderType", &col->type);
//
//             if (col->type == sm2d::ColliderType::sm2d_AABB)
//             {
//                 colliderEdited = ImGui::DragFloat2("Halfwidths",
//                 glm::value_ptr(col->aabb.halfwidths));
//             }
//             else if (col->type == sm2d::ColliderType::sm2d_Circle)
//             {
//                 colliderEdited = ImGui::DragFloat("Radius", &col->circle.radius);
//             }
//             else if (col->type == sm2d::ColliderType::sm2d_Polygon)
//             {
//                 std::string vertName = "Point 0";
//                 int         vertCounter = 0;
//                 for (int i = 0; i < col->polygon.points.size(); i++)
//                 {
//                     vertCounter++;
//                     vertName = vertName + std::to_string(vertCounter);
//                     colliderEdited =
//                         ImGui::DragFloat2(vertName.c_str(),
//                         glm::value_ptr(col->polygon.points[i]));
//                 }
//                 if (ImGui::Button("Add vertex"))
//                 {
//                     col->polygon.points.push_back(glm::vec2(0.0f, 0.0f));
//                 }
//                 if (ImGui::Button("Remove vertex"))
//                 {
//                     col->polygon.points.erase(col->polygon.points.begin() +
//                                               col->polygon.points.size() - 1);
//                 }
//             }
//
//             if (colliderEdited)
//             {
//                 if (col->type == sm2d::ColliderType::sm2d_AABB)
//                 {
//                     sm2d::RemoveLeaf(sm2d::bvh, col->treeIndex);
//                     sm2d::RemoveDeletedLeaves(sm2d::bvh);
//                     sm2d::InsertLeaf(sm2d::bvh, col, sm2d::ColAABBToABBB(*col));
//                 }
//                 else if (col->type == sm2d::ColliderType::sm2d_Circle)
//                 {
//                     sm2d::RemoveLeaf(sm2d::bvh, col->treeIndex);
//                     sm2d::RemoveDeletedLeaves(sm2d::bvh);
//                     sm2d::InsertLeaf(sm2d::bvh, col, sm2d::ColCircleToABBB(*col));
//                 }
//                 else if (col->type == sm2d::ColliderType::sm2d_Polygon)
//                 {
//                     if (col->polygon.worldPoints.empty())
//                     {
//                         for (int i = 0; i < col->polygon.points.size(); ++i)
//                         {
//                             col->polygon.worldPoints.push_back(glm::vec2(0.0f, 0.0f));
//                         }
//                     }
//                     sm2d::UpdatePolygon(*col);
//                     col->polygon.center = sm2d::ComputePolygonCenter(col->polygon);
//                     sm2d::RemoveLeaf(sm2d::bvh, col->treeIndex);
//                     sm2d::RemoveDeletedLeaves(sm2d::bvh);
//                     sm2d::InsertLeaf(sm2d::bvh, col, sm2d::ColPolygonToAABB(*col));
//                 }
//             }
//         }
//     }
//     // ----------------------------------
//     // TODO: Add particle systems pls someone
//
//     if (ImGui::BeginPopupContextWindow())
//     {
//         if (ImGui::MenuItem("Add SpriteRenderer"))
//         {
//             engineState.scene.Assign<SpriteRenderer>(selectedEntity);
//         }
//         if (ImGui::MenuItem("Add MeshRenderer"))
//         {
//             engineState.scene.AssignParam<MeshRenderer>(selectedEntity,
//                                                         Model("res/models/Box.obj"));
//         }
//         if (ImGui::MenuItem("Add RigidBody3D"))
//         {
//             engineState.scene.AssignParam<RigidBody3D>(selectedEntity, ColliderType::Box,
//                                                        BodyState::Dynamic);
//         }
//         if (ImGui::MenuItem("Add Animator"))
//         {
//             engineState.scene.Assign<Animator>(selectedEntity);
//         }
//         if (ImGui::MenuItem("Add Light"))
//         {
//             engineState.scene.Assign<Light>(selectedEntity);
//         }
//         if (ImGui::MenuItem("Add Rigidbody2D"))
//         {
//             engineState.scene.AssignParam<sm2d::Rigidbody>(
//                 selectedEntity, sm2d::BodyType::sm2d_Static,
//                 engineState.scene.Get<Transform>(selectedEntity));
//         }
//         if (ImGui::MenuItem("Add Collider2D"))
//         {
//             engineState.scene.AssignParam<sm2d::Collider>(
//                 selectedEntity, sm2d::sm2d_AABB, sm2d::ColAABB(glm::vec2(1.0f, 1.0f)),
//                 engineState.scene.Get<sm2d::Rigidbody>(selectedEntity));
//         }
//         ImGui::EndPopup();
//     }
//
//     ImGui::End();
// }
