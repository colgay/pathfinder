#include "extdll.h"
#include "AStar.h"

AStar::AStar()
{
}

AStar::~AStar()
{
}

void AStar::PushOpen(std::shared_ptr<AStarNode> pNode)
{
	m_open.push_back(pNode);
	pNode->SetOpen(true);
}

void AStar::PopOpen(std::shared_ptr<AStarNode> pNode)
{
	m_open.erase(std::remove(m_open.begin(), m_open.end(), pNode), m_open.end());
	pNode->SetOpen(true);
}

void AStar::PushClosed(std::shared_ptr<AStarNode> pNode)
{
	m_closed.push_back(pNode);
	pNode->SetClosed(true);
}

void AStar::ResetNodes(void)
{
	for (std::vector<std::shared_ptr<AStarNode>>::iterator it = m_open.begin(); it != m_open.end(); ++it)
	{
		(*it)->Reset();
	}

	for (std::vector<std::shared_ptr<AStarNode>>::iterator it = m_closed.begin(); it != m_closed.end(); ++it)
	{
		(*it)->Reset();
	}
}

void AStar::Reset(void)
{
	ResetNodes();
	m_open.clear();
	m_closed.clear();
}

const std::shared_ptr<AStarNode> AStar::GetNodeWithLowestF(void) const
{
	std::shared_ptr<AStarNode> pNode, pResult = nullptr;

	for (std::vector<std::shared_ptr<AStarNode>>::const_iterator it = m_open.cbegin(); it != m_open.cend(); ++it)
	{
		if (pResult == nullptr || pNode->GetF() < pResult->GetF())
			pResult = pNode;
	}

	return pResult;
}

bool AStar::CalcPath(std::shared_ptr<Node> pStart, const std::shared_ptr<Node> pGoal, Path &path)
{
	Reset();

	std::shared_ptr<AStarNode> pStart2 = std::static_pointer_cast<AStarNode>(pStart);
	std::shared_ptr<AStarNode> pGoal2 = std::static_pointer_cast<AStarNode>(pGoal);

	PushOpen(pStart2);
	
	float g;
	std::shared_ptr<AStarNode> pCurrent, pChild;
	const std::vector<std::shared_ptr<Children>> *children;

	while (!m_open.empty())
	{
		pCurrent = GetNodeWithLowestF();

		if (pCurrent == pGoal2)
		{
			Reconstruct(pCurrent, path);
			return true;
		}

		PopOpen(pCurrent);
		PushClosed(pCurrent);

		children = &pCurrent->GetChildren();

		for (std::vector<std::shared_ptr<Children>>::const_iterator it = children->cbegin(); it != children->cend(); ++it)
		{
			pChild = std::static_pointer_cast<AStarNode>((*it)->pNode);

			g = pChild->GetG() + pCurrent->DistBetween(pChild);
			if (!pChild->IsOpen())
				PushOpen(pChild);
			else if (g >= pChild->GetG())
				continue;

			pChild->SetParent((*it)->pNode);
			pChild->SetG(g);
			pChild->SetF(g + pChild->DistBetween(pGoal2));
		}
	}

	return false;
}

void AStar::Reconstruct(std::shared_ptr<AStarNode> pNode, Path &path)
{
	std::shared_ptr<AStarNode> pParent = std::static_pointer_cast<AStarNode>(pNode->GetParent());
	path.push_back(pNode);

	while (pParent != nullptr)
	{
		path.push_back(pParent);
		pParent = std::static_pointer_cast<AStarNode>(pNode->GetParent());
	}
}