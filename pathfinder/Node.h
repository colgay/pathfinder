#pragma once

#include <vector>
#include <memory>
#include <algorithm>

class Node;

struct Children
{
	std::shared_ptr<Node> pNode;
	int flags;
};

class Node
{
public:
	Node(const Vector &origin, float radius, int flags);
	virtual ~Node();

	const std::vector<std::shared_ptr<Children>> &GetChildren(void) const { return m_children; }
	const std::shared_ptr<Children> GetChild(std::shared_ptr<Node> pNode) const;
	float GetRadius(void) const { return m_radius; }
	int GetFlags(void) const { return m_flags; }
	const Vector &GetPosition(void) const { return m_position; }
	void AddChild(const std::shared_ptr<Node> pNode, int flags);
	void RemoveChild(const std::shared_ptr<Node> pNode);
	void SetRadius(float radius) { m_radius = radius; }
	void SetFlags(int flags) { m_flags = flags; }
	void SetPosition(const Vector &origin) { m_position = origin; }

private:
	std::vector<std::shared_ptr<Children>> m_children;
	float m_radius;
	int m_flags;
	Vector m_position;
};

