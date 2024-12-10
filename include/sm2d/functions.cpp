#include "colliders.h"
#include <sm2d/functions.h>
#include <cassert>

namespace sm2d
{

bool AABBEnlarge(AABB* a, const AABB& b)
{
    bool changed = false;
    if (b.lowerBound.x < a->lowerBound.x)
    {
        a->lowerBound.x = b.lowerBound.x;
        changed = true;
    }

    if (b.lowerBound.y < a->lowerBound.y)
    {
        a->lowerBound.y = b.lowerBound.y;
        changed = true;
    }

    if (a->upperBound.x < b.upperBound.x)
    {
        a->upperBound.x = b.upperBound.x;
        changed = true;
    }

    if (a->upperBound.y < b.upperBound.y)
    {
        a->upperBound.y = b.upperBound.y;
        changed = true;
    }

    return changed;
}

bool AABBTest(const AABB& a, const AABB& b)
{
    return !(b.lowerBound.x > a.upperBound.x || b.lowerBound.y > a.upperBound.y ||
             a.lowerBound.x > b.upperBound.x || a.lowerBound.y > b.upperBound.y);
}

float MinFloat(float a, float b)
{
    return a < b ? a : b;
}

float MaxFloat(float a, float b)
{
    return a > b ? a : b;
}

AABB AABBUnion(const AABB& a, const AABB& b)
{
    AABB c;
    c.lowerBound.x = MinFloat(a.lowerBound.x, b.lowerBound.x);
    c.lowerBound.y = MinFloat(a.lowerBound.y, b.lowerBound.y);
    c.upperBound.x = MaxFloat(a.upperBound.x, b.upperBound.x);
    c.upperBound.y = MaxFloat(a.upperBound.y, b.upperBound.y);
    return c;
}

float AABBPerimeter(const AABB& a)
{
    glm::vec2 d = a.upperBound - a.lowerBound; // Calculate width and height
    return 2.0f * (d.x + d.y);                 // Perimeter of the rectangle
}

glm::vec2 AABBCenter(const AABB& a)
{
    glm::vec2 b = {0.5f * (a.lowerBound.x + a.upperBound.x),
                   0.5f * (a.lowerBound.y + a.upperBound.y)};
    return b;
}

int FindBestSibling(Tree& tree, const AABB& box)
{
    float     boxArea = AABBPerimeter(box);
    glm::vec2 boxCenter = AABBCenter(box);
    float     baseArea = AABBPerimeter(tree.nodes[tree.rootIndex].box);

    int   currentNode = tree.rootIndex;
    int   bestSibling = currentNode;
    float directCost = AABBPerimeter(AABBUnion(tree.nodes[currentNode].box, box));
    float inheritedCost = 0.0f;
    float bestCost = directCost;

    while (!tree.nodes[currentNode].leaf)
    {
        float cost = directCost + inheritedCost;

        if (cost < bestCost)
        {
            bestCost = cost;
            bestSibling = currentNode;
        }

        inheritedCost += directCost - baseArea;

        int  child1 = tree.nodes[currentNode].child1;
        int  child2 = tree.nodes[currentNode].child2;
        bool leaf1 = tree.nodes[child1].leaf;
        bool leaf2 = tree.nodes[child2].leaf;

        // Cost of descending into child 1
        float lowerCost1 = FLT_MAX;
        AABB  box1 = tree.nodes[child1].box;
        float directCost1 = AABBPerimeter(AABBUnion(box1, box));
        float area1 = 0.0f;

        if (leaf1)
        {
            // Child 1 is a leaf
            // Cost of creating new node and increasing area of node P
            float cost1 = directCost1 + inheritedCost;

            // Need this here due to while condition above
            if (cost1 < bestCost)
            {
                bestSibling = child1;
                bestCost = cost1;
            }
        }
        else
        {
            // Child 1 is an internal node
            area1 = AABBPerimeter(box1);

            // Lower bound cost of inserting under child 1.
            lowerCost1 = inheritedCost + directCost1 + MinFloat(boxArea - area1, 0.0f);
        }

        // Cost of descending into child 2
        float lowerCost2 = FLT_MAX;
        AABB  box2 = tree.nodes[child2].box;
        float directCost2 = AABBPerimeter(AABBUnion(box2, box));
        float area2 = 0.0f;
        if (leaf2)
        {
            // Child 2 is a leaf
            // Cost of creating new node and increasing area of node P
            float cost2 = directCost2 + inheritedCost;

            // Need this here due to while condition above
            if (cost2 < bestCost)
            {
                bestSibling = child2;
                bestCost = cost2;
            }
        }
        else
        {
            // Child 2 is an internal node
            area2 = AABBPerimeter(box2);

            // Lower bound cost of inserting under child 2. This is not the cost
            // of child 2, it is the best we can hope for under child 2.
            lowerCost2 = inheritedCost + directCost2 + MinFloat(boxArea - area2, 0.0f);
        }

        if (leaf1 && leaf2)
        {
            break;
        }

        // Can the cost possibly be decreased?
        if (bestCost <= lowerCost1 && bestCost <= lowerCost2)
        {
            break;
        }

        if (lowerCost1 == lowerCost2 && leaf1 == false)
        {
            assert(lowerCost1 < FLT_MAX);
            assert(lowerCost2 < FLT_MAX);

            // No clear choice based on lower bound surface area. This can happen when both
            // children fully contain D. Fall back to node distance.
            glm::vec2 d1 = AABBCenter(box1) - boxCenter;
            glm::vec2 d2 = AABBCenter(box2) - boxCenter;
            lowerCost1 = glm::length2(d1);
            lowerCost2 = glm::length2(d2);
        }

        // Descend
        if (lowerCost1 < lowerCost2 && leaf1 == false)
        {
            currentNode = child1;
            baseArea = area1;
            directCost = directCost1;
        }
        else
        {
            currentNode = child2;
            baseArea = area2;
            directCost = directCost2;
        }

        assert(tree.nodes[currentNode].leaf == false && "Current node is not a leaf");
    }

    return bestSibling;
}

void InsertLeaf(Tree& tree, Collider* body, const AABB& box)
{
    // If the tree is empty, create the first node as the root
    if (tree.nodes.empty())
    {
        Node newNode;
        newNode.collider = body;
        newNode.box = box;
        newNode.index = 0;
        newNode.collider->treeIndex = 0;
        newNode.leaf = true;
        newNode.parentIndex = -1;
        newNode.child1 = -1;
        newNode.child2 = -1;

        tree.nodes.push_back(newNode);
        tree.rootIndex = 0;
        return;
    }

    // Stage 0: Push leaf to tree

    Node newNode;
    newNode.collider = body;
    newNode.box = box;
    newNode.index = (int)tree.nodes.size();
    newNode.collider->treeIndex = newNode.index;
    newNode.leaf = true;
    newNode.child1 = -1;
    newNode.child2 = -1;
    tree.nodes.push_back(newNode);

    int leaf = newNode.index;

    // Stage 1: Find best sibling for new leaf

    int sibling = FindBestSibling(tree, box);

    // Stage 2: Create a new parent

    int oldParent = tree.nodes[sibling].parentIndex;

    Node newParent;
    newParent.parentIndex = oldParent;
    newParent.box = AABBUnion(box, tree.nodes[sibling].box);
    newParent.leaf = false;
    newParent.collider = nullptr;
    newParent.index = (int)tree.nodes.size();

    tree.nodes.push_back(newParent);
    int newParentIndex = newParent.index;

    if (oldParent != -1)
    {
        // Sibling isn't the root
        if (tree.nodes[oldParent].child1 == sibling)
        {
            tree.nodes[oldParent].child1 = newParentIndex;
        }
        else
        {
            tree.nodes[oldParent].child2 = newParentIndex;
        }

        tree.nodes[newParentIndex].child1 = sibling;
        tree.nodes[newParentIndex].child2 = leaf;
        tree.nodes[sibling].parentIndex = newParentIndex;
        tree.nodes[leaf].parentIndex = newParentIndex;
    }
    else
    {
        // Sibling is the root
        tree.nodes[newParentIndex].child1 = sibling;
        tree.nodes[newParentIndex].child2 = leaf;
        tree.nodes[sibling].parentIndex = newParentIndex;
        tree.nodes[leaf].parentIndex = newParentIndex;
        tree.rootIndex = newParentIndex;
    }

    // Stage 3: Walk the tree backwards refitting parent AABBs
    int currentNode = newParentIndex;
    while (currentNode != -1)
    {
        Node& node = tree.nodes[currentNode];

        // Recompute the bounding box of the current node
        if (!node.leaf)
        {
            node.box = AABBUnion(tree.nodes[node.child1].box, tree.nodes[node.child2].box);
        }

        // Move up to the parent node
        currentNode = node.parentIndex;
    }
}

void RemoveLeaf(Tree& tree, int leafIndex)
{
    // If the tree is empty or the leaf index is invalid, do nothing
    if (tree.nodes.empty() || leafIndex < 0 || leafIndex >= tree.nodes.size())
        return;

    // If this is the root and it's a leaf, clear the entire tree
    if (leafIndex == tree.rootIndex && tree.nodes[leafIndex].leaf)
    {
        tree.nodes.clear();
        tree.rootIndex = -1;
        return;
    }

    // Get the parent of the leaf
    int parentIndex = tree.nodes[leafIndex].parentIndex;

    // If the leaf has no parent (shouldn't happen in a valid tree)
    if (parentIndex == -1)
        return;

    // Find the sibling of the leaf
    int grandParentIndex = tree.nodes[parentIndex].parentIndex;
    int siblingIndex = (tree.nodes[parentIndex].child1 == leafIndex)
                           ? tree.nodes[parentIndex].child2
                           : tree.nodes[parentIndex].child1;

    // If there's a grandparent, replace the parent with the sibling
    if (grandParentIndex != -1)
    {
        // Update the grandparent to point to the sibling
        if (tree.nodes[grandParentIndex].child1 == parentIndex)
        {
            tree.nodes[grandParentIndex].child1 = siblingIndex;
        }
        else
        {
            tree.nodes[grandParentIndex].child2 = siblingIndex;
        }

        // Update the sibling's parent
        tree.nodes[siblingIndex].parentIndex = grandParentIndex;
    }
    else
    {
        // The sibling becomes the new root
        tree.rootIndex = siblingIndex;
        tree.nodes[siblingIndex].parentIndex = -1;
    }

    tree.nodes[leafIndex] = Node(-1);
    tree.nodes[parentIndex] = Node(-1);

    // Refit the tree from the grandparent upwards
    int currentNode = grandParentIndex;
    while (currentNode != -1)
    {
        Node& node = tree.nodes[currentNode];

        // Recompute the bounding box of the current node
        if (!node.leaf)
        {
            node.box = AABBUnion(tree.nodes[node.child1].box, tree.nodes[node.child2].box);
        }

        // Move up to the parent node
        currentNode = node.parentIndex;
    }
}

void RemoveDeletedLeaves(Tree& tree)
{
    // First pass: create a mapping of old indices to new indices
    std::unordered_map<int, int> indexMap;
    std::vector<Node>            newNodes;

    for (int oldIndex = 0; oldIndex < tree.nodes.size(); ++oldIndex)
    {
        if (tree.nodes[oldIndex].index != -1)
        {
            indexMap[oldIndex] = (int)newNodes.size();
            newNodes.push_back(tree.nodes[oldIndex]);
            newNodes.back().index = (int)newNodes.size() - 1;
            if (newNodes.back().collider)
                newNodes.back().collider->treeIndex = (int)newNodes.size() - 1;
        }
    }

    // Second pass: update parent and child indices
    for (auto& node : newNodes)
    {
        if (node.parentIndex != -1)
        {
            node.parentIndex = indexMap[node.parentIndex];
        }

        if (node.child1 != -1)
        {
            node.child1 = indexMap[node.child1];
        }

        if (node.child2 != -1)
        {
            node.child2 = indexMap[node.child2];
        }
    }

    // Update root index
    tree.rootIndex = indexMap[tree.rootIndex];

    // Replace old nodes vector
    tree.nodes = std::move(newNodes);
}

void GetCollisionsInTree(const Tree& tree, std::vector<CollisionData>& collisionResults)
{
    // Recursive lambda function to traverse and check collisions
    std::function<void(int, int)> CheckCollisions = [&](int node1Index, int node2Index)
    {
        // Invalid node check
        if (node1Index == -1 || node2Index == -1)
            return;

        const Node& node1 = tree.nodes[node1Index];
        const Node& node2 = tree.nodes[node2Index];

        // First, check if the bounding boxes of the nodes overlap
        if (!AABBTest(node1.box, node2.box))
            return;

        // If both are leaf nodes, perform collision test
        if (node1.leaf && node2.leaf)
        {
            CollisionData data;
            if (node1.collider->type == ColliderType::sm2d_AABB &&
                node2.collider->type == ColliderType::sm2d_AABB)
            {
                data = TestColAABBAABB(*node1.collider, *node2.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_Circle &&
                     node2.collider->type == ColliderType::sm2d_Circle)
            {
                data = TestColCircleCircle(*node1.collider, *node2.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_Circle &&
                     node2.collider->type == ColliderType::sm2d_AABB)
            {
                data = TestColAABBCircle(*node2.collider, *node1.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_AABB &&
                     node2.collider->type == ColliderType::sm2d_Circle)
            {
                data = TestColAABBCircle(*node1.collider, *node2.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_OBB &&
                     node2.collider->type == ColliderType::sm2d_OBB)
            {
                data = TestColOBBOBB(*node1.collider, *node2.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_OBB &&
                     node2.collider->type == ColliderType::sm2d_AABB)
            {
                data = TestColAABBOBB(*node2.collider, *node1.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_AABB &&
                     node2.collider->type == ColliderType::sm2d_OBB)
            {
                data = TestColAABBOBB(*node1.collider, *node2.collider);
            }

            if (data)
                collisionResults.push_back(data);
            return;
        }

        // If one or both nodes are internal nodes, recurse further
        if (!node1.leaf && !node2.leaf)
        {
            // Check combinations of children for both internal nodes
            CheckCollisions(node1.child1, node2.child1);
            CheckCollisions(node1.child1, node2.child2);
            CheckCollisions(node1.child2, node2.child1);
            CheckCollisions(node1.child2, node2.child2);
        }
        else if (!node1.leaf)
        {
            // One node is internal, the other is a leaf
            CheckCollisions(node1.child1, node2Index);
            CheckCollisions(node1.child2, node2Index);
        }
        else // node2 is internal
        {
            CheckCollisions(node1Index, node2.child1);
            CheckCollisions(node1Index, node2.child2);
        }
    };

    // Start by checking the root node against itself and all other nodes
    CheckCollisions(tree.rootIndex, tree.rootIndex);
}

float CrossProduct(const glm::vec2& a, const glm::vec2& b)
{
    return a.x * b.y - a.y * b.x;
}

void ResolveCollisions(const Tree& tree, std::vector<CollisionData>& collisionResults)
{
    for (auto& colData : collisionResults)
    {
        Collider*  objectA = colData.objectA;
        Collider*  objectB = colData.objectB;
        Rigidbody* rigid1 = objectA->body;
        Rigidbody* rigid2 = objectB->body;

        // Positional correction
        float totalMass = rigid1->mass + rigid2->mass;
        if (totalMass > 0.0f)
        {
            glm::vec2 correctionA =
                -(colData.penetrationDepth / totalMass) * colData.collisionNormal * rigid2->mass;
            glm::vec2 correctionB =
                +(colData.penetrationDepth / totalMass) * colData.collisionNormal * rigid1->mass;

            if (rigid1->type == BodyType::sm2d_Dynamic)
                rigid1->transform->position += glm::vec3(correctionA, 0.0f);
            if (rigid2->type == BodyType::sm2d_Dynamic)
                rigid2->transform->position += glm::vec3(correctionB, 0.0f);
        }

        // Velocity adjustment
        glm::vec2 relativeVelocity = rigid2->linearVelocity - rigid1->linearVelocity;
        float     velocityAlongNormal = glm::dot(relativeVelocity, colData.collisionNormal);

        if (velocityAlongNormal < 0)
        {
            float e =
                std::min(rigid1->restitution, rigid2->restitution); // Coefficient of restitution
            float impulseMagnitude =
                -(1 + e) * velocityAlongNormal / (1.0f / rigid1->mass + 1.0f / rigid2->mass);

            glm::vec2 impulse = impulseMagnitude * colData.collisionNormal;
            if (rigid1->type == BodyType::sm2d_Dynamic)
                rigid1->linearVelocity -= impulse / (rigid1->mass);
            if (rigid2->type == BodyType::sm2d_Dynamic)
                rigid2->linearVelocity += impulse / (rigid2->mass);

            // Apply angular velocity changes
            glm::vec2 rA = colData.contactPoint - glm::vec2(rigid1->transform->position);
            glm::vec2 rB = colData.contactPoint - glm::vec2(rigid2->transform->position);

            if (rigid1->type == BodyType::sm2d_Dynamic && !rigid1->fixedRotation)
            {
                float torqueA = CrossProduct(rA, -impulse); // Torque due to impulse on A
                rigid1->angularVelocity -= torqueA / rigid1->momentOfInertia;
            }

            if (rigid2->type == BodyType::sm2d_Dynamic && !rigid2->fixedRotation)
            {
                float torqueB = CrossProduct(rB, impulse); // Torque due to impulse on B
                rigid2->angularVelocity += torqueB / rigid2->momentOfInertia;
            }
        }
    }
}

AABB ColAABBToABBB(const Collider& box)
{
    glm::vec2 topRight = glm::vec2(box.body->transform->position) +
                         glm::vec2(box.aabb.halfwidths.x, box.aabb.halfwidths.y);
    glm::vec2 bottomLeft = glm::vec2(box.body->transform->position) +
                           glm::vec2(-box.aabb.halfwidths.x, -box.aabb.halfwidths.y);
    return AABB(topRight, bottomLeft);
}

AABB ColCircleToABBB(const Collider& circle)
{
    glm::vec2 center = glm::vec2(circle.body->transform->position);
    glm::vec2 topRight = center + circle.circle.radius;
    glm::vec2 bottomLeft = center - circle.circle.radius;
    return AABB(topRight, bottomLeft);
}

AABB ColOBBToAABB(const Collider& obb)
{
    // Compute the cosine and sine of the rotation angle
    float cosTheta = cos(obb.body->transform->rotation.z);
    float sinTheta = sin(obb.body->transform->rotation.z);

    // The corners of the OBB in local space relative to its position
    glm::vec2 corners[4] = {
        {obb.obb.halfwidths.x, obb.obb.halfwidths.y},   // Top-right
        {obb.obb.halfwidths.x, -obb.obb.halfwidths.y},  // Bottom-right
        {-obb.obb.halfwidths.x, -obb.obb.halfwidths.y}, // Bottom-left
        {-obb.obb.halfwidths.x, obb.obb.halfwidths.y}   // Top-left
    };

    // Rotate the corners into world space and compute AABB bounds
    glm::vec2 minBounds = obb.body->transform->position; // Initialize min bounds
    glm::vec2 maxBounds = obb.body->transform->position; // Initialize max bounds

    for (int i = 0; i < 4; ++i)
    {
        // Rotate the corner
        glm::vec2 rotatedCorner = {
            obb.body->transform->position.x + corners[i].x * cosTheta - corners[i].y * sinTheta,
            obb.body->transform->position.y + corners[i].x * sinTheta + corners[i].y * cosTheta};

        // Update the min and max bounds
        minBounds = glm::min(minBounds, rotatedCorner);
        maxBounds = glm::max(maxBounds, rotatedCorner);
    }

    // Create the resulting AABB
    AABB aabb;
    aabb.lowerBound = minBounds;
    aabb.upperBound = maxBounds;

    return aabb;
}

} // namespace sm2d
