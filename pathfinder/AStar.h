#pragma once

#include "IPathFinder.h"
#include "Node.h"

class AStarNode : public Node
{
public:
	AStarNode(const Vector &origin, float radius, int flags)
		: Node(origin, radius, flags)
	{
	}

	float GetF(void) const { return m_f; }
	float GetG(void) const { return m_g; }
	float GetH(void) const { return m_h; }
	const std::shared_ptr<AStarNode> GetParent(void) const { return m_pParent; }
	bool IsOpen(void) const { return m_open; }
	bool IsClosed(void) const { return m_closed; }
	void SetF(float value) { m_f = value; }
	void SetG(float value) { m_g = value; }
	void SetH(float value) { m_h = value; }
	void SetParent(std::shared_ptr<AStarNode> pNode) { m_pParent = pNode; }
	void SetOpen(bool value) { m_open = value; }
	void SetClosed(bool value) { m_closed = value; }

	float DistBetween(std::shared_ptr<AStarNode> pNode)
	{
		return (this->GetPosition() - pNode->GetPosition()).Length();
	}

	void Reset(void)
	{
		m_open = m_closed = false;
		m_f = m_g = m_h = 0;
		m_pParent = nullptr;
	}

private:
	bool m_open, m_closed;
	float m_f, m_g, m_h;
	std::shared_ptr<AStarNode> m_pParent;
};

class AStar : public IPathFinder
{
public:
	AStar();
	virtual ~AStar();
	virtual bool CalcPath(const std::shared_ptr<Node> pStart, const std::shared_ptr<Node> pGoal, Path &path) override;
	virtual void AssignMap(Map *pMap) override { m_pMap = pMap; }
	virtual void Reset(void) override;

private:
	void ResetNodes(void);
	void PushOpen(std::shared_ptr<AStarNode> pNode);
	void PopOpen(std::shared_ptr<AStarNode> pNode);
	void PushClosed(std::shared_ptr<AStarNode> pNode);
	const std::shared_ptr<AStarNode> GetNodeWithLowestF(void) const;
	void Reconstruct(std::shared_ptr<AStarNode> pNode, Path &path);

	virtual std::shared_ptr<Node> CreateNode(const Vector &origin, float radius, int flags)
	{
		return std::shared_ptr<Node>(new AStarNode(origin, radius, flags));
	}

	Map *m_pMap;
	std::vector<std::shared_ptr<AStarNode>> m_open, m_closed;
};

