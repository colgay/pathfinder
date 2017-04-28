#pragma once

#include <vector>
#include <memory>

class Node;
class IPathFinder;

class Map
{
public:
	Map();
	~Map();

	const std::shared_ptr<Node> CreateNode(IPathFinder *pPathFinder, const Vector &origin, float radius, int flags);
	void RemoveNode(std::shared_ptr<Node> pNode);
	const std::shared_ptr<Node> GetNodeAt(int index) const;
	int GetNodeIndex(std::shared_ptr<Node> pNode) const;
	void SetNodes(std::vector<std::shared_ptr<Node>> *pNodes) { m_nodes = *pNodes; }
	const std::vector<std::shared_ptr<Node>> &GetNodes(void) const { return m_nodes; }
	const std::shared_ptr<Node> GetNearestNode(const Vector &origin, float maxRadius) const;
	void Clear(void) { m_nodes.clear(); }

private:
	std::vector<std::shared_ptr<Node>> m_nodes;
};