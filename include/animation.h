#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <bone.h>
#include <functional>
#include <model.h>

// Courtesy of https://learnopengl.com

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

	Animation(const std::string& animationPath, Model* model);

	~Animation();

	Bone* FindBone(const std::string& name);
	
	float GetTicksPerSecond();

	float GetDuration();

	const AssimpNodeData& GetRootNode();

	const std::map<std::string,BoneInfo>& GetBoneIDMap();

private:
	void ReadMissingBones(const aiAnimation* animation, Model& model);

	void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);

	float duration;
	int ticksPerSecond;
	std::vector<Bone> bones;
	AssimpNodeData rootNode;
	std::map<std::string, BoneInfo> boneInfoMap;
};