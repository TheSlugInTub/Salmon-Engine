#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <bone.h>
#include <functional>
#include <model.h>

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

class Animation
{
public:
	Animation() = default;

	Animation(const std::string& animationPath, Model* model)
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

	~Animation()
	{
	}

	Bone* FindBone(const std::string& name)
	{
		auto iter = std::find_if(bones.begin(), bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == bones.end()) return nullptr;
		else return &(*iter);
	}

	
	inline float GetTicksPerSecond() { return (float)ticksPerSecond; }
	inline float GetDuration() { return duration;}
	inline const AssimpNodeData& GetRootNode() { return rootNode; }
	inline const std::map<std::string,BoneInfo>& GetBoneIDMap() 
	{ 
		return boneInfoMap;
	}

private:
	void ReadMissingBones(const aiAnimation* animation, Model& model)
	{
		int size = animation->mNumChannels;

		auto& lboneInfoMap = model.GetBoneInfoMap();//getting boneInfoMap from Model class

                int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (lboneInfoMap.find(boneName) == lboneInfoMap.end())
			{
				lboneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			bones.push_back(Bone(channel->mNodeName.data,
				lboneInfoMap[channel->mNodeName.data].id, channel));
		}

		boneInfoMap = lboneInfoMap;
	}

	void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (int i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHierarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}
	float duration;
	int ticksPerSecond;
	std::vector<Bone> bones;
	AssimpNodeData rootNode;
	std::map<std::string, BoneInfo> boneInfoMap;
};

