#include <sm2d/functions.h>
#include <cassert>
#include <cmath>
#include <algorithm>

namespace sm2d
{

std::optional<glm::vec2> GetLineIntersection(const glm::vec2& p0, const glm::vec2& p1,
                                             const glm::vec2& q0, const glm::vec2& q1)
{
    glm::vec2 s1 = p1 - p0;
    glm::vec2 s2 = q1 - q0;

    float s, t;
    float denom = (-s2.x * s1.y + s1.x * s2.y);

    if (denom == 0)
    {
        // Lines are parallel or collinear
        return std::nullopt;
    }

    s = (-s1.y * (p0.x - q0.x) + s1.x * (p0.y - q0.y)) / denom;
    t = (s2.x * (p0.y - q0.y) - s2.y * (p0.x - q0.x)) / denom;

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        // Intersection detected
        return p0 + t * s1;
    }

    // No intersection
    return std::nullopt;
}

glm::vec2 ClosestPointOnLineSegment(const glm::vec2& vertex, const glm::vec2& lineStart,
                                    const glm::vec2& lineEnd)
{
    // Vector from lineStart to lineEnd
    glm::vec2 lineVector = lineEnd - lineStart;
    // Vector from lineStart to the vertex
    glm::vec2 vertexVector = vertex - lineStart;

    float lineLengthSquared = glm::dot(lineVector, lineVector);

    // Avoid division by zero in case of a degenerate line segment
    if (lineLengthSquared == 0.0f)
    {
        return lineStart; // Line segment is a point
    }

    // Compute the projection factor (t) of the vertex onto the line
    float t = glm::dot(vertexVector, lineVector) / lineLengthSquared;

    // Clamp t to the range [0, 1] to find the closest point on the segment
    t = std::clamp(t, 0.0f, 1.0f);

    // Calculate the closest point on the line segment
    return lineStart + t * lineVector;
}

glm::vec2 LocalToWorld(glm::vec2 point, const glm::vec2 pos, float cosine, float sine)
{
    float x = (cosine * point.x - sine * point.y) + pos.x;
    float y = (sine * point.x + cosine * point.y) + pos.y;
    return glm::vec2(x, y);
}

void UpdatePolygon(Collider& poly)
{
    float sine = sin(poly.body->transform->rotation.z);
    float cosine = cos(poly.body->transform->rotation.z);

    glm::vec2 pos = glm::vec2(poly.body->transform->position);

    if (poly.polygon.worldPoints.size() != poly.polygon.points.size())
    {
        poly.polygon.worldPoints.resize(poly.polygon.points.size());
    }
    for (int i = 0; i < poly.polygon.points.size(); ++i)
    {
        poly.polygon.worldPoints[i] = LocalToWorld(poly.polygon.points[i], pos, cosine, sine);
    }
}

glm::vec2 ComputePolygonCenter(ColPolygon& poly)
{
    glm::vec2 center = glm::vec2(0.0f, 0.0f);
    float     area = 0.0f;

    // Get a reference point for forming triangles.
    // Use the first vertex to reduce round-off errors.
    glm::vec2 origin = poly.worldPoints[0];

    const float inv3 = 1.0f / 3.0f;

    for (int i = 1; i < poly.worldPoints.size() - 1; ++i)
    {
        // Triangle edges
        glm::vec2 e1 = poly.worldPoints[i] - origin;
        glm::vec2 e2 = poly.worldPoints[i + 1] - origin;
        float     a = 0.5f * CrossProduct(e1, e2);

        // Area weighted centroid
        center = center + (a * inv3) * (e1 + e2);
        area += a;
    }

    assert(area < FLT_EPSILON);
    float invArea = 1.0f / area;
    center.x *= invArea;
    center.y *= invArea;

    // Restore offset
    center = origin + center;

    return center;
}

void ComputeAABBPoints(const Collider& collider, std::vector<glm::vec2>& points)
{
    glm::vec2 topLeft = glm::vec2(collider.body->transform->position) +
                        glm::vec2(-collider.aabb.halfwidths.x, collider.aabb.halfwidths.y);
    glm::vec2 topRight = glm::vec2(collider.body->transform->position) +
                         glm::vec2(collider.aabb.halfwidths.x, collider.aabb.halfwidths.y);
    glm::vec2 bottomRight = glm::vec2(collider.body->transform->position) +
                            glm::vec2(collider.aabb.halfwidths.x, -collider.aabb.halfwidths.y);
    glm::vec2 bottomLeft = glm::vec2(collider.body->transform->position) +
                           glm::vec2(-collider.aabb.halfwidths.x, -collider.aabb.halfwidths.y);
    points = {bottomLeft, topLeft, topRight, bottomRight};
}

glm::vec2 VectorScalarCross(const glm::vec2& v, float s)
{
    return glm::vec2(s * v.y, -s * v.x);
}

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

glm::vec2 ClosestPointOnAABB(const glm::vec2& point, const Collider& aabb)
{
    glm::vec2 localPoint = point - glm::vec2(aabb.body->transform->position);

    // Clamp the point to the AABB's bounds
    glm::vec2 closestLocal = glm::clamp(localPoint, -aabb.aabb.halfwidths, aabb.aabb.halfwidths);

    // Transform back to world space
    glm::vec2 worldClosest = closestLocal + glm::vec2(aabb.body->transform->position);

    return worldClosest;
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

void GetCollisionsInTree(const Tree& tree, std::vector<Manifold>& collisionResults)
{
    if (bvh.nodes.empty())
        return;
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
            Manifold data;
            if (node1.collider->type == ColliderType::sm2d_AABB &&
                node2.collider->type == ColliderType::sm2d_AABB)
            {
                data = TestColAABBAABB(*node1.collider, *node2.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_Polygon &&
                     node2.collider->type == ColliderType::sm2d_Polygon)
            {
                data = TestColPolygonPolygon(*node1.collider, *node2.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_Polygon &&
                     node2.collider->type == ColliderType::sm2d_AABB)
            {
                data = TestColAABBPolygon(*node2.collider, *node1.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_AABB &&
                     node2.collider->type == ColliderType::sm2d_Polygon)
            {
                data = TestColAABBPolygon(*node1.collider, *node2.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_Polygon &&
                     node2.collider->type == ColliderType::sm2d_Circle)
            {
                data = TestColCirclePolygon(*node2.collider, *node1.collider);
            }
            else if (node1.collider->type == ColliderType::sm2d_Circle &&
                     node2.collider->type == ColliderType::sm2d_Polygon)
            {
                data = TestColCirclePolygon(*node1.collider, *node2.collider);
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

            if (data)
            {
                bool node1Moved = node1.collider->body->hasMoved &&
                                  !node1.collider->body->type == BodyType::sm2d_Static;
                bool node2Moved = node2.collider->body->hasMoved &&
                                  !node2.collider->body->type == BodyType::sm2d_Static;

                if (node1Moved && !node2Moved)
                {
                    node2.collider->body->awake = true;
                }
                else if (node2Moved && !node1Moved)
                {
                    node1.collider->body->awake = true;
                }
                else if (!node1Moved && !node2Moved)
                {
                    // Skip pushing the result if neither has moved
                    node1.collider->body->awake = false;
                    node2.collider->body->awake = false;
                    return;
                }

                collisionResults.push_back(data);
            }
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

void ResolveCollisions(const Tree& tree, std::vector<Manifold>& collisionResults)
{
    for (auto& colData : collisionResults)
    {
        Collider*  objectA = colData.objectA;
        Collider*  objectB = colData.objectB;
        Rigidbody* rigid1 = objectA->body;
        Rigidbody* rigid2 = objectB->body;

        // Position correction
        float totalMass = rigid1->mass + rigid2->mass;
        if (totalMass > 0.0f)
        {
            const float penetrationTolerance = 0.005f; // Adjust this value as needed
            float       correctionMagnitude = std::max(0.0f,
                                                       colData.penetrationDepth - penetrationTolerance) *
                                        0.8f; // Bias factor

            glm::vec2 correctionA =
                -(correctionMagnitude / totalMass) * colData.collisionNormal * rigid2->mass;
            glm::vec2 correctionB =
                +(correctionMagnitude / totalMass) * colData.collisionNormal * rigid1->mass;

            if (rigid1->type == BodyType::sm2d_Dynamic)
                rigid1->transform->position += glm::vec3(correctionA, 0.0f);
            if (rigid2->type == BodyType::sm2d_Dynamic)
                rigid2->transform->position += glm::vec3(correctionB, 0.0f);
        }

        // Velocity resolution
        glm::vec2 rA = colData.contactPoint - glm::vec2(rigid1->transform->position);
        glm::vec2 rB = colData.contactPoint - glm::vec2(rigid2->transform->position);

        glm::vec2 relativeVelocity =
            rigid2->linearVelocity +
            glm::vec2(-rigid2->angularVelocity * rB.y, rigid2->angularVelocity * rB.x) -
            rigid1->linearVelocity -
            glm::vec2(-rigid1->angularVelocity * rA.y, rigid1->angularVelocity * rA.x);

        float velocityAlongNormal = glm::dot(relativeVelocity, colData.collisionNormal);

        if (velocityAlongNormal < 0)
        {
            float e = std::min(rigid1->restitution, rigid2->restitution);

            // Calculate angular contributions
            float rACrossN = CrossProduct(rA, colData.collisionNormal);
            float rBCrossN = CrossProduct(rB, colData.collisionNormal);

            float angularFactor = (rACrossN * rACrossN) / rigid1->momentOfInertia +
                                  (rBCrossN * rBCrossN) / rigid2->momentOfInertia;

            // Dampen angular impulse for vertex collisions
            float vertexCollisionDamping = 0.7f; // Adjust this value to control angular damping
            angularFactor *= vertexCollisionDamping;

            float impulseMagnitude = -(1 + e) * velocityAlongNormal /
                                     ((1.0f / rigid1->mass + 1.0f / rigid2->mass) + angularFactor);

            glm::vec2 impulse = impulseMagnitude * colData.collisionNormal;

            // Apply linear impulses
            if (rigid1->type == BodyType::sm2d_Dynamic)
                rigid1->linearVelocity -= impulse / rigid1->mass;
            if (rigid2->type == BodyType::sm2d_Dynamic)
                rigid2->linearVelocity += impulse / rigid2->mass;

            // Apply angular impulses with damping
            if (rigid1->type == BodyType::sm2d_Dynamic && !rigid1->fixedRotation)
            {
                float torqueA = CrossProduct(rA, -impulse);
                rigid1->angularVelocity +=
                    (torqueA / rigid1->momentOfInertia) * vertexCollisionDamping;
            }

            if (rigid2->type == BodyType::sm2d_Dynamic && !rigid2->fixedRotation)
            {
                float torqueB = CrossProduct(rB, impulse);
                rigid2->angularVelocity +=
                    (torqueB / rigid2->momentOfInertia) * vertexCollisionDamping;
            }
        }
    }
}

void UpdateCollider(Collider* collider)
{
    if (collider->type == ColliderType::sm2d_AABB)
    {
        RemoveLeaf(bvh, collider->treeIndex);
        RemoveDeletedLeaves(bvh);
        InsertLeaf(bvh, collider, ColAABBToABBB(*collider));
    }
    else if (collider->type == ColliderType::sm2d_Circle)
    {
        RemoveLeaf(bvh, collider->treeIndex);
        RemoveDeletedLeaves(bvh);
        InsertLeaf(bvh, collider, ColCircleToABBB(*collider));
    }
    else if (collider->type == ColliderType::sm2d_Polygon)
    {
        UpdatePolygon(*collider);
        collider->polygon.center = ComputePolygonCenter(collider->polygon);
        RemoveLeaf(bvh, collider->treeIndex);
        RemoveDeletedLeaves(bvh);
        InsertLeaf(bvh, collider, ColPolygonToAABB(*collider));
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

AABB ColPolygonToAABB(const Collider& poly)
{
    glm::vec2 upperBound = glm::vec2(poly.body->transform->position);
    glm::vec2 lowerBound = upperBound;
    for (auto& point : poly.polygon.worldPoints)
    {
        upperBound.x = MaxFloat(point.x, upperBound.x);
        upperBound.y = MaxFloat(point.y, upperBound.y);
        lowerBound.x = MinFloat(point.x, lowerBound.x);
        lowerBound.y = MinFloat(point.y, lowerBound.y);
    }
    return AABB(upperBound, lowerBound);
}

} // namespace sm2d
