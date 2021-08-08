#pragma once
#define MAX_NODE_POINTS 2000
#define GPS_LINE_WIDTH  5.0f * 0.5f
#define MAX_TARGET_DISTANCE 10.0f

class CSprite2d;

class CGPS {
public:
    static bool bShowGPS;
    static float fGPSDistance;
    static char nPathDirection;
    static short nNodesCount;
    static CNodeAddress resultNodes[MAX_NODE_POINTS];
    static CVector2D nodePoints[MAX_NODE_POINTS];
    static CRGBA highlightColor;
    static CSprite2d pathDirSprite;
    static CVector vecDest;
    static bool bDestFound;
    static unsigned int pathColor;

public:
    CGPS();
    static void Init();
    static void Shutdown();
    static void DrawDistanceFromWaypoint();
    static void DrawPathLine();
    static void FindMissionMarker();
    static void CalculateRoute(CVector vec);
    static void FindNearestObjective();
    static void SetRoute(CVector vec);
    static void DrawLine(CVector2D const& a, CVector2D const& b, float width, CRGBA color);
    static char GetPathDirection(CVector start, CVector plr, CVector end);
};

float FindClosest(float* array, int size, int& id);
