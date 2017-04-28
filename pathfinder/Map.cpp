#include "extdll.h"
#include "Map.h"
#include "Node.h"
#include "IPathFinder.h"

Map::Map()
{
}


Map::~Map()
{
}

const std::shared_ptr<Node> Map::CreateNode(IPathFinder *pPathFinder, const Vector &origin, float radius, int flags)
{
	std::shared_ptr<Node> pNode(pPathFinder->CreateNode(origin, radius, flags));
	m_nodes.push_back(pNode);
	
	return pNode;
}

void Map::RemoveNode(std::shared_ptr<Node> pNode)
{
	m_nodes.erase(std::remove(m_nodes.begin(), m_nodes.end(), pNode), m_nodes.end());
}

int Map::GetNodeIndex(std::shared_ptr<Node> pNode) const
{
	std::vector<std::shared_ptr<Node>>::const_iterator it = std::find(m_nodes.cbegin(), m_nodes.cend(), pNode);
	
	return std::distance(m_nodes.cbegin(), it);
}

const std::shared_ptr<Node> Map::GetNearestNode(const Vector &origin, float maxRadius) const
{
	std::shared_ptr<Node> pNode, pResult = nullptr;
	float minRadius = maxRadius;

	for (std::vector<std::shared_ptr<Node>>::const_iterator it = m_nodes.cbegin(); it != m_nodes.cend(); ++it)
	{
		pNode = *it;

		float dist = (pNode->GetPosition() - origin).Length();
		if (dist < minRadius)
		{
			pResult = pNode;
			minRadius = dist;
		}
	}

	return pResult;
}

const std::shared_ptr<Node> Map::GetNodeAt(int index) const
{
	if (index >= m_nodes.size())
		return nullptr;

	return m_nodes.at(index);
}