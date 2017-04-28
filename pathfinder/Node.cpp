#include "extdll.h"
#include "Node.h"

Node::Node(const Vector &origin, float radius, int flags)
{
	m_position = origin;
	m_radius = radius;
	m_flags = flags;
}


Node::~Node()
{
}

const std::shared_ptr<Children> Node::GetChild(std::shared_ptr<Node> pNode) const
{
	std::vector<std::shared_ptr<Children>>::const_iterator it;

	it = std::find_if(m_children.cbegin(), m_children.cend(), 
		[pNode](std::shared_ptr<Children> child) {
			return child->pNode == pNode;
		}
	);

	if (it == m_children.cend())
		return nullptr;

	return *it;
}

void Node::AddChild(const std::shared_ptr<Node> pNode, int flags)
{
	std::shared_ptr<Children> pChild = std::make_shared<Children>();
	pChild->pNode = pNode;
	pChild->flags = flags;

	m_children.push_back(pChild);
}

void Node::RemoveChild(const std::shared_ptr<Node> pNode)
{
	auto it = std::remove_if(m_children.begin(), m_children.end(),
		[pNode](std::shared_ptr<Children> child) {
			return child->pNode == pNode;
		}
	);

	m_children.erase(it, m_children.end());
}