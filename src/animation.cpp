#pragma once

#include <salmon/animation.h>

// Courtesy of https://learnopengl.com

Animation::Animation(const std::string& animationPath, Model* model)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    auto animation = scene->mAnimations[0];
    duration = (float)animation->mDuration;
    ticksPerSecond = (int)animation->mTicksPerSecond;
    aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
    globalTransformation = globalTransformation.Inverse();
    ReadHierarchyData(rootNode, scene->mRootNode);
    ReadMissingBones(animation, *model);
}

Animation::~Animation()
{
}

Bone* Animation::FindBone(const std::string& name)
{
    auto iter = std::find_if(bones.begin(), bones.end(), [&](const Bone& Bone) { return Bone.GetBoneName() == name; });
    if (iter == bones.end())
        return nullptr;
    else
        return &(*iter);
}

float Animation::GetTicksPerSecond()
{
    return (float)ticksPerSecond;
}

float Animation::GetDuration()
{
    return duration;
}

const AssimpNodeData& Animation::GetRootNode()
{
    return rootNode;
}

const std::map<std::string, BoneInfo>& Animation::GetBoneIDMap()
{
    return boneInfoMap;
}

void Animation::ReadMissingBones(const aiAnimation* animation, Model& model)
{
    int size = animation->mNumChannels;

    auto& lboneInfoMap = model.GetBoneInfoMap(); // getting boneInfoMap from Model class

    int& boneCount = model.GetBoneCount(); // getting the m_BoneCounter from Model class

    // reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (lboneInfoMap.find(boneName) == lboneInfoMap.end())
        {
            lboneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        bones.push_back(Bone(channel->mNodeName.data, lboneInfoMap[channel->mNodeName.data].id, channel));
    }

    boneInfoMap = lboneInfoMap;
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
    assert(src);

    dest.name = src->mName.data;
    dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < (int)src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}
