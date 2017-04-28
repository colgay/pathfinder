#pragma once

#include <vector>
#include <memory>

class Node;
class Map;

char* UTIL_VarArgs(const char *format, ...);

void UTIL_ClientPrintAll(int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4);

void UTIL_HudMessage(edict_t *pEntity, const hudtextparms_t &textparms, const char *pMessage);

std::shared_ptr<Node> UTIL_GetAimNode(edict_t *pEntity, float maxDist, Map *pMap);

float UTIL_DistLineSegments(Vector s1p0, Vector s1p1, Vector s2p0, Vector s2p1);

float UTIL_Distance(const Vector &a, const Vector &b);

float UTIL_Dot(const Vector &a, const Vector &b);