#pragma once

#include <vector>
#include <memory>

class Node;
class Map;

typedef std::vector<std::shared_ptr<Node>> Path;

class IPathFinder
{
public:
	virtual ~IPathFinder() {}

	virtual bool CalcPath(const std::shared_ptr<Node> start, const std::shared_ptr<Node> goal, Path &path) = 0;
	virtual void AssignMap(Map *map) = 0;
	virtual void Reset(void) = 0;

	virtual std::shared_ptr<Node> CreateNode(const Vector &origin, float radius, int flags) = 0;
};

