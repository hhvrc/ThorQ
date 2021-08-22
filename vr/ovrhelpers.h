#ifndef OVRHELPERS_H
#define OVRHELPERS_H

#include <QMatrix4x4>

inline void ToQMatrix(const vr::HmdMatrix34_t& mat, QMatrix4x4& out)
{
    for (int i = 0; i < 3; i++)
        out.setColumn(i, QVector4D(mat.m[i][0], mat.m[i][1], mat.m[i][2], mat.m[i][3]));
}
inline void ToHmdMatrix34(const QMatrix4x4& mat, vr::HmdMatrix34_t& out)
{
    for (int i = 0; i < 3; i++)
    {
        QVector4D col = mat.column(i);
        out.m[i][0] = col.x();
        out.m[i][1] = col.y();
        out.m[i][2] = col.z();
        out.m[i][3] = col.w();
    }
}

#endif // OVRHELPERS_H
