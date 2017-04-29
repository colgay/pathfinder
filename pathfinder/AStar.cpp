#include "extdll.h"
#include "AStar.h"
#include "utilities.h"

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
		pNode = *it;
		if (pResult == nullptr || pNode->GetF() < pResult->GetF())
			pResult = pNode;
	}

	return pResult;
}

bool AStar::CalcPath(const std::shared_ptr<Node> pStart, const std::shared_ptr<Node> pGoal, Path &path)
{
	Reset();

	std::shared_ptr<AStarNode> pStart2 = std::dynamic_pointer_cast<AStarNode>(pStart);
	std::shared_ptr<AStarNode> pGoal2 = std::dynamic_pointer_cast<AStarNode>(pGoal);

	const std::vector<std::shared_ptr<Children>> *children;
	std::shared_ptr<AStarNode> pCurrent, pChild;
	std::shared_ptr<Node> pNode;
	float f, g, h;

	PushOpen(pStart2);

	while (!m_open.empty())
	{
		pCurrent = GetNodeWithLowestF();
		PopOpen(pCurrent);
		PushClosed(pCurrent);
		UTIL_ClientPrintAll(HUD_PRINTCONSOLE, UTIL_VarArgs("is ... %p ?\n", pCurrent), "", "", "", "");

		if (pCurrent == pGoal2)
		{
			Reconstruct(pCurrent, path);
			return true;
		}

		children = &pCurrent->GetChildren();
		for (std::vector<std::shared_ptr<Children>>::const_iterator it = children->cbegin(); it != children->cend(); ++it)
		{
			pNode = (*it)->pNode;
			pChild = std::dynamic_pointer_cast<AStarNode>(pNode);
			
			g = pCurrent->GetG() + pCurrent->DistBetween(pChild);
			if ((pChild->IsOpen() || pChild->IsClosed()) && pChild->GetG() < g)
				continue;

			h = pChild->DistBetween(pGoal2);
			f = g + h;
			
			pChild->SetF(f);
			pChild->SetG(g);
			pChild->SetH(h);
			pChild->SetParent(pCurrent);

			if (pChild->IsClosed())
				pChild->SetClosed(false);
			if (!pChild->IsOpen())
				PushOpen(pChild);
		}
	}

	return false;
}

void AStar::Reconstruct(std::shared_ptr<AStarNode> pNode, Path &path)
{
	std::shared_ptr<AStarNode> pParent = pNode->GetParent();
	path.push_back(std::static_pointer_cast<Node>(pNode));

	while (pParent != nullptr)
	{
		path.push_back(std::static_pointer_cast<Node>(pParent));
		pParent = pParent->GetParent();
	}
}