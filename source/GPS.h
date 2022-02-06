#pragma once
#define NODE_MULTIPLIER 10
#define MAX_PATH_NODES 5000 * NODE_MULTIPLIER
#define MAX_SEARCH_RADIUS 6000.0f

#define MAX_NODE_POINTS 2000
#define MAX_TARGET_DISTANCE 10.0f

class CSprite2d;

enum ePathDir {
    DIR_RIGHT,
    DIR_FORWARD,
    DIR_LEFT,
    DIR_NONE = 8
};

struct CLocalization {
    bool bDestFound;
    CVector vecDest;
    float fGPSDistance;
    char nPathDirection;
    short nNodesCount;
    CNodeAddress resultNodes[MAX_NODE_POINTS];
    CVector2D nodePoints[MAX_NODE_POINTS];
    unsigned int pathColor;

    void Clear();
};

class CGPS {
public:
    static CLocalization Dest;
    static bool bShowGPS;
    static CSprite2d pathDirSprite;

public:
    static void Init();
    static void Shutdown();
    static void DrawDistanceFromWaypoint();
    static void DrawPathLine();

    static void DrawLine(CVector2D const& a, CVector2D const& b, float width, CRGBA color);
    static void ProcessPath(CLocalization& l);
    static char GetPathDirection(CVector start, CVector plr, CVector end);
};

