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
        if (!IsEntityValid(engineState.scene.entities[i].id))
            continue;

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

    if (ImGui::BeginPopupContextWindow())
    {
        ComponentRegistry::Instance().AddAll();
        ImGui::EndPopup();
    }

    ImGui::End();
}

void SaveScene(const std::string& filename)
{
    std::filesystem::path directory = "res/scenes";
    // Ensure the directory exists
    std::filesystem::create_directories(directory);

    std::filesystem::path filePath = directory / filename;

    nlohmann::json j;
    ComponentRegistry::Instance().SaveAll(j);

    std::ofstream file(filePath);
    file << j.dump(4);
}

void LoadScene(const std::string& filename)
{
    std::filesystem::path directory = "res/scenes";
    std::filesystem::path filePath = directory / filename;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << '\n';
        return;
    }

    nlohmann::json j;
    file >> j;

    engineState.scene.entities.clear();
    engineState.scene.freeEntities.clear();
    engineState.scene.componentPools.clear();

    ComponentRegistry::Instance().LoadAll(j);
}

void DrawTray()
{
    ImGui::Begin("Tray");

    char sceneBuffer[128];
    strncpy_s(sceneBuffer, sceneName.c_str(), sizeof(sceneBuffer));
    if (ImGui::InputText("SceneName", sceneBuffer, sizeof(sceneBuffer),
                         ImGuiInputTextFlags_EnterReturnsTrue))
    {
        sceneName = std::string(sceneBuffer);
    }

    if (ImGui::Button("Save"))
    {
        SaveScene(sceneName);
    }
    if (ImGui::Button("Load"))
    {
        LoadScene(sceneName);
    }
    if (!playing)
    {
        if (ImGui::Button("Play"))
        {
            SaveScene(sceneName);
            playing = true;
            StartStartSystems();
        }
    }
    else
    {
        if (ImGui::Button("Stop"))
        {
            LoadScene(sceneName);
            playing = false;
        }
    }
    ImGui::End();
}
