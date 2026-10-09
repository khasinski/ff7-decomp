//! PSYQ=3.3

#include "highway_private.h"

// The four view frustum corner rays at the projection distance.
const VECTOR g_HighwayFrustumBottomLeft = {-160, 120, 300, 0};

const VECTOR g_HighwayFrustumBottomRight = {160, 120, 300, 0};

const VECTOR g_HighwayFrustumTopLeft = {-160, -120, 300, 0};

const VECTOR g_HighwayFrustumTopRight = {160, -120, 300, 0};

void HighwayFrustumInit(void) {
    VECTOR tl;
    VECTOR bl;
    VECTOR tr;
    VECTOR br;
    VECTOR blCorner;
    VECTOR brCorner;
    VECTOR tlCorner;
    VECTOR trCorner;
    s32 lx;
    s32 ly;
    s32 lz;
    s32 rx;
    s32 ry;
    s32 rz;

    blCorner = g_HighwayFrustumBottomLeft;
    brCorner = g_HighwayFrustumBottomRight;
    tlCorner = g_HighwayFrustumTopLeft;
    trCorner = g_HighwayFrustumTopRight;

    tl.vx = tlCorner.vx >> 2;
    tl.vy = tlCorner.vy >> 2;
    tl.vz = tlCorner.vz >> 2;
    bl.vx = blCorner.vx >> 2;
    bl.vy = blCorner.vy >> 2;
    bl.vz = blCorner.vz >> 2;
    tr.vx = trCorner.vx >> 2;
    tr.vy = trCorner.vy >> 2;
    tr.vz = trCorner.vz >> 2;
    br.vx = brCorner.vx >> 2;
    br.vy = brCorner.vy >> 2;
    br.vz = brCorner.vz >> 2;
    OuterProduct0(&tl, &bl, &g_HighwayLeftPlaneNormal);
    OuterProduct0(&tr, &br, &g_HighwayRightPlaneNormal);

    lx = g_HighwayLeftPlaneNormal.vx;
    ly = g_HighwayLeftPlaneNormal.vy;
    lz = g_HighwayLeftPlaneNormal.vz;
    rx = g_HighwayRightPlaneNormal.vx;
    ry = g_HighwayRightPlaneNormal.vy;
    rz = g_HighwayRightPlaneNormal.vz;
    g_HighwayLeftPlaneDistance = -((tlCorner.vx >> 2) * lx) - ((tlCorner.vy >> 2) * ly) - ((tlCorner.vz >> 2) * lz);
    g_HighwayRightPlaneDistance = -((trCorner.vx >> 2) * rx) - ((trCorner.vy >> 2) * ry) - ((trCorner.vz >> 2) * rz);
    g_HighwayLeftPlaneInsideRef =
        ((trCorner.vx >> 2) * lx) + ((trCorner.vy >> 2) * ly) + ((trCorner.vz >> 2) * lz) + g_HighwayLeftPlaneDistance;
    g_HighwayRightPlaneInsideRef =
        ((tlCorner.vx >> 2) * rx) + ((tlCorner.vy >> 2) * ry) + ((tlCorner.vz >> 2) * rz) + g_HighwayRightPlaneDistance;
    g_HighwayLeftNormalLength = SquareRoot0((lx * lx) + (ly * ly) + (lz * lz));
    g_HighwayRightNormalLength = SquareRoot0(
        (g_HighwayRightPlaneNormal.vx * g_HighwayRightPlaneNormal.vx) +
        (g_HighwayRightPlaneNormal.vy * g_HighwayRightPlaneNormal.vy) +
        (g_HighwayRightPlaneNormal.vz * g_HighwayRightPlaneNormal.vz));
}

s32 HighwayVectorInsidePlanes(VECTOR* point) {
    s32 hsLeft;
    s32 rightOk;
    s32 leftOk;
    s32 hsRight;

    leftOk = 0;
    rightOk = 0;
    hsLeft = (point->vx >> 2) * g_HighwayLeftPlaneNormal.vx + (point->vy >> 2) * g_HighwayLeftPlaneNormal.vy +
             (point->vz >> 2) * g_HighwayLeftPlaneNormal.vz + g_HighwayLeftPlaneDistance;
    hsRight = (point->vx >> 2) * g_HighwayRightPlaneNormal.vx + (point->vy >> 2) * g_HighwayRightPlaneNormal.vy +
              (point->vz >> 2) * g_HighwayRightPlaneNormal.vz + g_HighwayRightPlaneDistance;
    if (hsLeft > 0 && g_HighwayLeftPlaneInsideRef > 0) {
        leftOk = 1;
    }
    if (hsLeft < 0 && g_HighwayLeftPlaneInsideRef < 0) {
        leftOk = 1;
    }
    if (hsRight > 0 && g_HighwayRightPlaneInsideRef > 0) {
        rightOk = 1;
    }
    if (hsRight < 0 && g_HighwayRightPlaneInsideRef < 0) {
        rightOk = 1;
    }
    return leftOk & rightOk;
}

s32 HighwaySVectorInsidePlanes(SVECTOR* point) {
    s32 hsLeft;
    s32 rightOk;
    s32 leftOk;
    s32 hsRight;

    leftOk = 0;
    rightOk = 0;
    hsLeft = (point->vx >> 2) * g_HighwayLeftPlaneNormal.vx + (point->vy >> 2) * g_HighwayLeftPlaneNormal.vy +
             (point->vz >> 2) * g_HighwayLeftPlaneNormal.vz + g_HighwayLeftPlaneDistance;
    hsRight = (point->vx >> 2) * g_HighwayRightPlaneNormal.vx + (point->vy >> 2) * g_HighwayRightPlaneNormal.vy +
              (point->vz >> 2) * g_HighwayRightPlaneNormal.vz + g_HighwayRightPlaneDistance;
    if (hsLeft > 0 && g_HighwayLeftPlaneInsideRef > 0) {
        leftOk = 1;
    }
    if (hsLeft < 0 && g_HighwayLeftPlaneInsideRef < 0) {
        leftOk = 1;
    }
    if (hsRight > 0 && g_HighwayRightPlaneInsideRef > 0) {
        rightOk = 1;
    }
    if (hsRight < 0 && g_HighwayRightPlaneInsideRef < 0) {
        rightOk = 1;
    }
    return leftOk & rightOk;
}

s32 HighwayLeftPlaneHalfSpace(s32 x, s32 y, s32 z) {
    return (x >> 2) * g_HighwayLeftPlaneNormal.vx + (y >> 2) * g_HighwayLeftPlaneNormal.vy +
           (z >> 2) * g_HighwayLeftPlaneNormal.vz + g_HighwayLeftPlaneDistance;
}

s32 HighwayRightPlaneHalfSpace(s32 x, s32 y, s32 z) {
    return (x >> 2) * g_HighwayRightPlaneNormal.vx + (y >> 2) * g_HighwayRightPlaneNormal.vy +
           (z >> 2) * g_HighwayRightPlaneNormal.vz + g_HighwayRightPlaneDistance;
}

s32 HighwaySphereInsidePlanes(VECTOR* center, s16 radius) {
    s32 leftOk;
    s32 hsLeft;
    s32 rightOk;
    s32 hsRight;
    s32 planeDistance;
    s32 len;

    leftOk = 0;
    rightOk = 0;
    hsLeft = (center->vx >> 2) * g_HighwayLeftPlaneNormal.vx + (center->vy >> 2) * g_HighwayLeftPlaneNormal.vy +
             (center->vz >> 2) * g_HighwayLeftPlaneNormal.vz + g_HighwayLeftPlaneDistance;
    if (g_HighwayLeftPlaneInsideRef > 0 && hsLeft >= 0) {
        leftOk = 1;
    }
    if (g_HighwayLeftPlaneInsideRef < 0 && hsLeft <= 0) {
        leftOk = 1;
    }
    if (!leftOk) {
        len = g_HighwayLeftNormalLength;
        planeDistance = ((hsLeft < 0) ? -hsLeft : hsLeft) / len;
        if (planeDistance < radius) {
            leftOk = 1;
        }
    }
    hsRight = (center->vx >> 2) * g_HighwayRightPlaneNormal.vx + (center->vy >> 2) * g_HighwayRightPlaneNormal.vy +
              (center->vz >> 2) * g_HighwayRightPlaneNormal.vz + g_HighwayRightPlaneDistance;
    if (g_HighwayRightPlaneInsideRef > 0 && hsRight >= 0) {
        rightOk = 1;
    }
    if (g_HighwayRightPlaneInsideRef < 0 && hsRight <= 0) {
        rightOk = 1;
    }
    if (!rightOk) {
        len = g_HighwayRightNormalLength;
        planeDistance = ((hsRight < 0) ? -hsRight : hsRight) / len;
        if (planeDistance < radius) {
            rightOk = 1;
        }
    }
    return leftOk & rightOk;
}

s32 HighwaySphereInsideLeftPlane(s32 x, s32 y, s32 z, s16 radius) {
    s32 hs;
    s32 ok;
    s32 len;

    ok = 0;
    hs = (x >> 2) * g_HighwayLeftPlaneNormal.vx + (y >> 2) * g_HighwayLeftPlaneNormal.vy +
         (z >> 2) * g_HighwayLeftPlaneNormal.vz + g_HighwayLeftPlaneDistance;
    if (g_HighwayLeftPlaneInsideRef > 0 && hs >= 0) {
        ok = 1;
    }
    if (g_HighwayLeftPlaneInsideRef < 0 && hs <= 0) {
        ok = 1;
    }
    if (!ok) {
        len = g_HighwayLeftNormalLength;
        if (hs < 0) {
            hs = -hs;
        }
        if (hs / len < radius) {
            ok = 1;
        }
    }
    return ok;
}

s32 HighwaySphereInsideRightPlane(s32 x, s32 y, s32 z, s16 radius) {
    s32 hs;
    s32 ok;
    s32 len;

    ok = 0;
    hs = (x >> 2) * g_HighwayRightPlaneNormal.vx + (y >> 2) * g_HighwayRightPlaneNormal.vy +
         (z >> 2) * g_HighwayRightPlaneNormal.vz + g_HighwayRightPlaneDistance;
    if (g_HighwayRightPlaneInsideRef > 0 && hs >= 0) {
        ok = 1;
    }
    if (g_HighwayRightPlaneInsideRef < 0 && hs <= 0) {
        ok = 1;
    }
    if (!ok) {
        len = g_HighwayRightNormalLength;
        if (hs < 0) {
            hs = -hs;
        }
        if (hs / len < radius) {
            ok = 1;
        }
    }
    return ok;
}
