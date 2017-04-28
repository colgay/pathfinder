#include "amxxmodule.h"
#include "Map.h"
#include "AStar.h"
#include "utilities.h"

static Map s_map;
static AStar s_pathfinder;

edict_t *g_pEditor = nullptr;
short g_spriteZBeam1;
short g_spriteZBeam4;
short g_spriteArrow;

static cell AMX_NATIVE_CALL wp_GetNearestNode(AMX *amx, cell *params)
{
	cell *addr = MF_GetAmxAddr(amx, params[1]);
	Vector origin = Vector(amx_ctof(addr[0]), amx_ctof(addr[1]), amx_ctof(addr[2]));
	REAL radius = amx_ctof(params[2]);

	std::shared_ptr<Node> node = s_map.GetNearestNode(origin, radius);

	if (node == nullptr)
		return -1;

	return s_map.GetNodeIndex(node);
}

static cell AMX_NATIVE_CALL wp_GetAimNode(AMX *amx, cell *params)
{
	int entityId = params[1];
	REAL radius = amx_ctof(params[2]);

	edict_t *entity = INDEXENT(entityId);

	std::shared_ptr<Node> node = UTIL_GetAimNode(entity, radius, &s_map);

	if (node == nullptr)
		return -1;

	return s_map.GetNodeIndex(node);
}

AMX_NATIVE_INFO pathfinder_natives[] =
{
	{ "wp_GetNearestNode", wp_GetNearestNode },
	{ "wp_GetAimNode", wp_GetAimNode },
	{ NULL, NULL }
};

void OnPluginsLoaded(void)
{
	g_spriteZBeam1 = PRECACHE_MODEL("sprites/zbeam1.spr");
	g_spriteZBeam4 = PRECACHE_MODEL("sprites/zbeam4.spr");
	g_spriteArrow = PRECACHE_MODEL("sprites/arrow1.spr");
}

void OnAmxxAttach(void)
{
	MF_AddNatives(pathfinder_natives);
}

void OnAmxxDetach(void)
{
	s_pathfinder.Reset();
	s_map.Clear();
}

void OnClientCommand(edict_t *pEntity)
{
	const char *szCmd = CMD_ARGV(0);

	if (strcmp(szCmd, "wp") == 0)
	{
		szCmd = CMD_ARGV(1);

		if (strcmp(szCmd, "editor") == 0)
		{
			if (atoi(CMD_ARGV(2)))
			{
				g_pEditor = pEntity;
				UTIL_ClientPrintAll(HUD_PRINTTALK, "* Waypoint editor is ON.\n");
			}
			else
			{
				g_pEditor = nullptr;
				UTIL_ClientPrintAll(HUD_PRINTTALK, "* Waypoint editor is OFF.\n");
			}

			RETURN_META(MRES_SUPERCEDE);
		}
		
		if (g_pEditor != pEntity)
			RETURN_META(MRES_IGNORED);

		if (strcmp(szCmd, "create") == 0)
		{
			float radius = atof(CMD_ARGV(2));
			int flags = atoi(CMD_ARGV(3));

			std::shared_ptr<Node> pNode = s_map.CreateNode(&s_pathfinder, pEntity->v.origin, radius, flags);
			if (pNode != nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Create waypoint #%d.\n", s_map.GetNodeIndex(pNode)));
			}

			RETURN_META(MRES_SUPERCEDE);
		}
		else if (strcmp(szCmd, "remove") == 0)
		{
			int num = atoi(CMD_ARGV(2));
			std::shared_ptr<Node> pNode = s_map.GetNodeAt(num);

			if (pNode == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find waypoint #%d.\n", num));
				RETURN_META(MRES_SUPERCEDE);
			}

			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Remove waypoint #%d.\n", s_map.GetNodeIndex(pNode)));
			s_map.RemoveNode(pNode);

			RETURN_META(MRES_SUPERCEDE);
		}
		else if (strcmp(szCmd, "addchild") == 0)
		{
			int num1, num2, flags;
			num1 = atoi(CMD_ARGV(2));
			num2 = atoi(CMD_ARGV(3));
			flags = atoi(CMD_ARGV(4));

			std::shared_ptr<Node> pNode, pOther;
			pNode = s_map.GetNodeAt(num1);
			pOther = s_map.GetNodeAt(num2);

			if (pNode == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find waypoint #%d.\n", num1));
				RETURN_META(MRES_SUPERCEDE);
			}

			if (pOther == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find waypoint #%d.\n", num2));
				RETURN_META(MRES_SUPERCEDE);
			}

			if (pNode == pOther || pNode->GetChild(pOther) != nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Unable to add child #%d to #%d.\n", num2, num1));
				RETURN_META(MRES_SUPERCEDE);
			}

			pNode->AddChild(pOther, flags);
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Add child #%d to #%d.\n", num2, num1));
			RETURN_META(MRES_SUPERCEDE);
		}
		else if (strcmp(szCmd, "popchild") == 0)
		{
			int num1, num2;
			num1 = atoi(CMD_ARGV(2));
			num2 = atoi(CMD_ARGV(3));

			std::shared_ptr<Node> pNode, pOther;
			pNode = s_map.GetNodeAt(num1);
			pOther = s_map.GetNodeAt(num2);

			if (pNode == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find waypoint #%d.\n", num1));
				RETURN_META(MRES_SUPERCEDE);
			}

			if (pOther == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find waypoint #%d.\n", num2));
				RETURN_META(MRES_SUPERCEDE);
			}

			if (pNode->GetChild(pOther) == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find child #%d from #%d.\n", num2, num1));
				RETURN_META(MRES_SUPERCEDE);
			}

			pNode->RemoveChild(pOther);
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Remove child #%d from #%d.\n", num2, num1));
			RETURN_META(MRES_SUPERCEDE);
		}
		else if (strcmp(szCmd, "setchildflags") == 0)
		{
			int num1, num2, flags;
			num1 = atoi(CMD_ARGV(2));
			num2 = atoi(CMD_ARGV(3));
			flags = atoi(CMD_ARGV(4));

			std::shared_ptr<Node> pNode, pOther;
			pNode = s_map.GetNodeAt(num1);
			pOther = s_map.GetNodeAt(num2);

			if (pNode == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find waypoint #%d.\n", num1));
				RETURN_META(MRES_SUPERCEDE);
			}

			if (pOther == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find waypoint #%d.\n", num2));
				RETURN_META(MRES_SUPERCEDE);
			}

			std::shared_ptr<Children> pChild = pNode->GetChild(pOther);
			if (pChild == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find child #%d from #%d.\n", num2, num1));
				RETURN_META(MRES_SUPERCEDE);
			}

			pChild->flags = flags;
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Add child flag to #%d from #%d.\n", num2, num1));
			RETURN_META(MRES_SUPERCEDE);
		}
		else if (strcmp(szCmd, "setflags") == 0)
		{
			int num, flags;
			num = atoi(CMD_ARGV(2));
			flags = atoi(CMD_ARGV(3));

			std::shared_ptr<Node> pNode;
			pNode = s_map.GetNodeAt(num);

			if (pNode == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find waypoint #%d.\n", num));
				RETURN_META(MRES_SUPERCEDE);
			}

			pNode->SetFlags(flags);
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Set flag for waypoint #%d.\n", num));
			RETURN_META(MRES_SUPERCEDE);
		}
		else if (strcmp(szCmd, "setradius") == 0)
		{
			int num = atoi(CMD_ARGV(2));
			float radius = atoi(CMD_ARGV(3));

			std::shared_ptr<Node> pNode;
			pNode = s_map.GetNodeAt(num);

			if (pNode == nullptr)
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Cannot find waypoint #%d.\n", num));
				RETURN_META(MRES_SUPERCEDE);
			}

			pNode->SetRadius(radius);
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("* Set radius(%.f) for waypoint #%d.\n", radius, num));
			RETURN_META(MRES_SUPERCEDE);
		}
	}

	RETURN_META(MRES_IGNORED);
}

void OnPlayerPreThink(edict_t *pEntity)
{
	if (g_pEditor == pEntity)
	{
		static float lastUpdateTime = 0.0f;
		if (gpGlobals->time < lastUpdateTime + 0.5)
			RETURN_META(MRES_IGNORED);

		Vector origin = pEntity->v.origin;

		// There is no nodes
		if (s_map.GetNodes().size() < 1)
			RETURN_META(MRES_IGNORED);

		Map map = s_map;

		std::vector<std::shared_ptr<Node>> nodes = map.GetNodes();

		// Sort the nodes
		std::sort(nodes.begin(), nodes.end(),
			[origin](const std::shared_ptr<Node> left, const std::shared_ptr<Node> right) {
				return UTIL_Distance(origin, left->GetPosition()) < UTIL_Distance(origin, right->GetPosition());
			}
		);

		map.SetNodes(&nodes);

		const std::vector<std::shared_ptr<Children>> *pChildren;
		std::shared_ptr<Node> pNode, pChild, pAimNode, pCurrent;

		pCurrent = map.GetNearestNode(origin, 100.0);
		pAimNode = UTIL_GetAimNode(pEntity, 50.0f, &map);

		int color[3];
		int newEnd = min(nodes.size(), 60);

		for (std::vector<std::shared_ptr<Node>>::const_iterator it = nodes.cbegin(); it != nodes.cbegin() + newEnd; ++it)
		{
			 pNode = *it;

			if (pCurrent == pNode)
			{
				color[0] = 255;
				color[1] = color[2] = 0;
				
				hudtextparms_t textparms;
				textparms.x = 0.3;
				textparms.y = 0.25;
				textparms.fadeinTime = 0.0;
				textparms.holdTime = 1.0;
				textparms.fadeoutTime = 1.0;
				textparms.r1 = 0;
				textparms.g1 = 200;
				textparms.b1 = 50;
				textparms.r2 = 255;
				textparms.g2 = 255;
				textparms.b2 = 255;
				textparms.channel = 3;

				UTIL_HudMessage(pEntity, textparms, UTIL_VarArgs("Waypoint #%d\nXYZ: {%.3f, %.3f, %.3f}\nRadius: %.f", 
					s_map.GetNodeIndex(pNode),
					pNode->GetPosition().x, pNode->GetPosition().y, pNode->GetPosition().z,
					pNode->GetRadius())
				);
			}
			else if (pAimNode == pNode)
			{
				color[0] = color[1] = color[2] = 200;

				MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pEntity);
				WRITE_BYTE(TE_BEAMPOINTS);
				WRITE_COORD(pEntity->v.origin.x);
				WRITE_COORD(pEntity->v.origin.y);
				WRITE_COORD(pEntity->v.origin.z);
				WRITE_COORD(pNode->GetPosition().x);
				WRITE_COORD(pNode->GetPosition().y);
				WRITE_COORD(pNode->GetPosition().z);
				WRITE_SHORT(g_spriteArrow);
				WRITE_BYTE(0);		// framerate
				WRITE_BYTE(0);		// framerate
				WRITE_BYTE(5);		// life
				WRITE_BYTE(10);		// width
				WRITE_BYTE(0);		// noise
				WRITE_BYTE(200);	// r
				WRITE_BYTE(200);		// g
				WRITE_BYTE(200);		// b
				WRITE_BYTE(200);	// brightness
				WRITE_BYTE(3);		// speed
				MESSAGE_END();
			}
			else
			{
				color[1] = 255;
				color[0] = color[2] = 0;
			}

			// Show a waypoint
			MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pEntity);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(pNode->GetPosition().x);
			WRITE_COORD(pNode->GetPosition().y);
			WRITE_COORD(pNode->GetPosition().z - 36);
			WRITE_COORD(pNode->GetPosition().x);
			WRITE_COORD(pNode->GetPosition().y);
			WRITE_COORD(pNode->GetPosition().z + 36);
			WRITE_SHORT(g_spriteZBeam4);
			WRITE_BYTE(0);		// framerate
			WRITE_BYTE(0);		// framerate
			WRITE_BYTE(5);		// life
			WRITE_BYTE(10);		// width
			WRITE_BYTE(0);		// noise
			WRITE_BYTE(color[0]);	// r
			WRITE_BYTE(color[1]);		// g
			WRITE_BYTE(color[2]);		// b
			WRITE_BYTE(200);	// brightness
			WRITE_BYTE(0);		// speed
			MESSAGE_END();

			pChildren = &pNode->GetChildren();

			for (std::vector<std::shared_ptr<Children>>::const_iterator it_c = pChildren->cbegin(); it_c != pChildren->cend(); ++it_c)
			{
				pChild = (*it_c)->pNode;

				if (pChild->GetChild(pNode) != nullptr)
				{
					color[0] = color[1] = 200;
					color[2] = 0;
				}
				else
				{
					color[0] = 200;
					color[1] = 50;
					color[2] = 0;
				}

				MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pEntity);
				WRITE_BYTE(TE_BEAMPOINTS);
				WRITE_COORD(pNode->GetPosition().x);
				WRITE_COORD(pNode->GetPosition().y);
				WRITE_COORD(pNode->GetPosition().z);
				WRITE_COORD(pChild->GetPosition().x);
				WRITE_COORD(pChild->GetPosition().y);
				WRITE_COORD(pChild->GetPosition().z);
				WRITE_SHORT(g_spriteZBeam1);
				WRITE_BYTE(0);		// framerate
				WRITE_BYTE(0);		// framerate
				WRITE_BYTE(5);		// life
				WRITE_BYTE(10);		// width
				WRITE_BYTE(3);		// noise
				WRITE_BYTE(color[0]);	// r
				WRITE_BYTE(color[1]);		// g
				WRITE_BYTE(color[2]);		// b
				WRITE_BYTE(200);	// brightness
				WRITE_BYTE(0);		// speed
				MESSAGE_END();
			}
		}

		lastUpdateTime = gpGlobals->time;
	}

	RETURN_META(MRES_IGNORED);
}