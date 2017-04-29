#include "extdll.h"
#include "meta_api.h"

#include "utilities.h"
#include "Node.h"
#include "Map.h"

extern short g_spriteZBeam4;

char* UTIL_VarArgs(const char *format, ...)
{
	va_list		argptr;
	static char		string[1024];

	va_start(argptr, format);
	vsprintf(string, format, argptr);
	va_end(argptr);

	return string;
}

void UTIL_ClientPrintAll(int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4)
{
	MESSAGE_BEGIN(MSG_BROADCAST, GET_USER_MSG_ID(PLID, "TextMsg", nullptr));
	WRITE_BYTE(msg_dest);
	WRITE_STRING(msg_name);

	if (param1)
		WRITE_STRING(param1);
	if (param2)
		WRITE_STRING(param2);
	if (param3)
		WRITE_STRING(param3);
	if (param4)
		WRITE_STRING(param4);

	MESSAGE_END();
}

void UTIL_HudMessage(edict_t *pEntity, const hudtextparms_t &textparms, const char *pMessage)
{
	if (pEntity == nullptr)
		return;

	MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pEntity);
	WRITE_BYTE(TE_TEXTMESSAGE);
	WRITE_BYTE(textparms.channel & 0xFF);
	WRITE_SHORT(FixedSigned16(textparms.x, 1 << 13));
	WRITE_SHORT(FixedSigned16(textparms.y, 1 << 13));
	WRITE_BYTE(textparms.effect);
	WRITE_BYTE(textparms.r1);
	WRITE_BYTE(textparms.g1);
	WRITE_BYTE(textparms.b1);
	WRITE_BYTE(textparms.a1);
	WRITE_BYTE(textparms.r2);
	WRITE_BYTE(textparms.g2);
	WRITE_BYTE(textparms.b2);
	WRITE_BYTE(textparms.a2);
	WRITE_SHORT(FixedUnsigned16(textparms.fadeinTime, 1 << 8));
	WRITE_SHORT(FixedUnsigned16(textparms.fadeoutTime, 1 << 8));
	WRITE_SHORT(FixedUnsigned16(textparms.holdTime, 1 << 8));

	if (textparms.effect == 2)
		WRITE_SHORT(FixedUnsigned16(textparms.fxTime, 1 << 8));

	if (!pMessage)
		WRITE_STRING(" ");	//TODO: oh yeah
	else
	{
		if (strlen(pMessage) >= 512)
		{
			char tmp[512];
			strncpy(tmp, pMessage, 511);
			tmp[511] = 0;
			WRITE_STRING(tmp);
		}
		else
			WRITE_STRING(pMessage);
	}
	MESSAGE_END();
}

std::shared_ptr<Node> UTIL_GetAimNode(edict_t *pEntity, float maxDist, Map *pMap)
{
	Vector start = pEntity->v.origin + pEntity->v.view_ofs;
	Vector end;
	MAKE_VECTORS(pEntity->v.v_angle);
	end = start + (gpGlobals->v_forward * 4096);

	const std::vector<std::shared_ptr<Node>> *pNodes = &pMap->GetNodes();
	std::shared_ptr<Node> pNode, pResult = nullptr;
	Vector pos1, pos2;
	float dist, minDist = maxDist;

	for (std::vector<std::shared_ptr<Node>>::const_iterator it = pNodes->cbegin(); it != pNodes->cend(); ++it)
	{
		pNode = *it;

		pos1 = pos2 = pNode->GetPosition();
		pos1.z -= 36.0;
		pos2.z += 36.0;

		dist = UTIL_DistLineSegments(start, end, pos1, pos2);
		if (dist < minDist)
		{
			minDist = dist;
			pResult = pNode;
		}
	}

	return pResult;
}

float UTIL_DistLineSegments(Vector s1p0, Vector s1p1, Vector s2p0, Vector s2p1)
{
	static const float SMALL_NUM = 0.00000001;

	Vector u = s1p1 - s1p0;
	Vector v = s2p1 - s2p0;
	Vector w = s1p0 - s2p0;

	float a = UTIL_Dot(u, u); // always >= 0
	float b = UTIL_Dot(u, v);
	float c = UTIL_Dot(v, v); // always >= 0
	float d = UTIL_Dot(u, w);
	float e = UTIL_Dot(v, w);
	float D = a*c - b*b; // always >= 0
	float sc, sN, sD = D; // sc = sN / sD, default sD = D >= 0
	float tc, tN, tD = D; // tc = tN / tD, default tD = D >= 0

	// compute the line parameters of the two closest points
	if (D < SMALL_NUM) // the lines are almost parallel
	{
		sN = 0.0; // force using point P0 on segment S1
		sD = 1.0; // to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else // get the closest points on the infinite lines
	{
		sN = (b*e - c*d);
		tN = (a*e - b*d);
		if (sN < 0.0) // sc < 0 => the s=0 edge is visible
		{
			sN = 0.0;
			tN = e;
			tD = c;
		}
		else if (sN > sD) // sc > 1  => the s=1 edge is visible
		{
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0.0) // tc < 0 => the t=0 edge is visible
	{
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
			sN = 0.0;
		else if (-d > a)
			sN = sD;
		else
		{
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD) // tc > 1  => the t=1 edge is visible
	{
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
			sN = 0;
		else if ((-d + b) > a)
			sN = sD;
		else
		{
			sN = (-d + b);
			sD = a;
		}
	}

	// finally do the division to get sc and tc
	sc = (abs(sN) < SMALL_NUM ? 0.0 : sN / sD);
	tc = (abs(tN) < SMALL_NUM ? 0.0 : tN / tD);

	Vector dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)
	return dP.Length();
}

float UTIL_Distance(const Vector &vec1, const Vector &vec2)
{
	return (vec1 - vec2).Length();
}

float UTIL_Dot(const Vector &vec1, const Vector &vec2)
{
	return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}