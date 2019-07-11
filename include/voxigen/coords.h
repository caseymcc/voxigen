#ifndef _voxigen_coords_h_
#define _voxigen_coords_h_

#include "voxigen/voxigen_export.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <math.h>
#include <vector>

namespace voxigen
{

//Notes:
//Coordinate system for this library is right-handed x,y plane with z as up. Theta is treated as azimuth and phi as inclination

inline glm::vec3 getCylindricalCoords(size_t width, size_t height, const glm::vec3 &position)
{
    glm::vec3 cylindricalPos;

    float theta=glm::two_pi<float>()*(float)(position.x/width);

    cylindricalPos.x=position.z*cos(theta);
    cylindricalPos.y=position.z*sin(theta);
    cylindricalPos.z=position.y;

    return cylindricalPos;
}

inline glm::vec3 getSphericalCoords(size_t width, size_t height, const glm::vec3 &position)
{
    glm::vec3 cylindricalPos;

    float phi=glm::half_pi<float>()-(glm::pi<float>()*(position.y/height));
    float theta=glm::two_pi<float>()*(position.x/width);

    cylindricalPos.z=position.z*sin(phi);
    float d=position.z*cos(phi);
    cylindricalPos.x=d*cos(theta);
    cylindricalPos.y=d*sin(theta);
    

    return cylindricalPos;
}

inline void equirectangularToSpherical(const std::vector<glm::vec2> &inPoints, float radius, std::vector<glm::vec3> &outPoints)
{
    if(outPoints.size()!=inPoints.size())
        outPoints.resize(inPoints.size());

    for(size_t i=0; i<inPoints.size(); ++i)
    {
        const glm::vec2 &inPoint=inPoints[i];
        glm::vec3 &outPoint=outPoints[i];
        
        float theta=glm::two_pi<float>()*(inPoint.x);
        float phi=glm::half_pi<float>()-(glm::pi<float>()*(inPoint.y));
        float d=radius*cos(phi);

        outPoint.x=d*cos(theta);
        outPoint.y=d*sin(theta);
        outPoint.z=sin(phi);//assuming unit sphere 1.0f*
    }
}

inline void cartesianToSpherical(const std::vector<glm::vec3> &inPoints, std::vector<glm::vec3> &outPoints)
{
    if(outPoints.size()!=inPoints.size())
        outPoints.resize(inPoints.size());

    for(size_t i=0; i<inPoints.size(); ++i)
    {
        const glm::vec3 &inPoint=inPoints[i];
        glm::vec3 &outPoint=outPoints[i];

        float d=sqrt((inPoint.x*inPoint.x)+(inPoint.y*inPoint.y));
        
        outPoint.x=sqrt((d*d)+(inPoint.z*inPoint.z)); //radius
        outPoint.y=acos(inPoint.x/d); //azimuth, theta
        outPoint.z=atan(inPoint.z/d); //inclination, phi
    }
}

inline std::vector<glm::vec3> cartesianToSpherical(const std::vector<glm::vec3> &inPoints)
{
    std::vector<glm::vec3> outPoints;

    cartesianToSpherical(inPoints, outPoints);
    return outPoints;
}

inline void sphericalToCartesian(const std::vector<glm::vec3> &inPoints, std::vector<glm::vec3> &outPoints)
{
    if(outPoints.size()!=inPoints.size())
        outPoints.resize(inPoints.size());

    for(size_t i=0; i<inPoints.size(); ++i)
    {
        const glm::vec3 &inPoint=inPoints[i];
        glm::vec3 &outPoint=outPoints[i];

        float d=inPoint.x*cos(inPoint.z);

        outPoint.x=d*cos(inPoint.y);
        outPoint.y=d*sin(inPoint.y);
        outPoint.z=inPoint.x*sin(inPoint.z);
    }
}

inline std::vector<glm::vec3> sphericalToCartesian(const std::vector<glm::vec3> &inPoints)
{
    std::vector<glm::vec3> outPoints;

    sphericalToCartesian(inPoints, outPoints);
    return outPoints;
}

//Generates a unit sphere using the golden section so that nodes in nearby bands are not too close, return format is in sphereical coordinates (radius, azimuth, inclination) or (r, theta, phi)
inline void generateFibonacciSphere(size_t count, std::vector<glm::vec3> &points)
{
    if(points.size() != count)
        points.resize(count);

    float longitudeDelta=glm::pi<float>()*(3.0f-sqrt(5.0f));  //~2.39996323 

    float dz=2.0f/count;
    float z=1.0f-(dz/2.0f);

    float longitude=0.0f;

    for(size_t k=0; k<count; k++)
    {
        glm::vec3 &point=points[k];

        float d=sqrt(1.0f-z*z);

        point.x=1.0f; //radius
        point.y=longitude; //azimuth, theta
        point.z=asin(z); //inclination, phi

        z-=dz;
        longitude+=longitudeDelta;
    }
}

inline std::vector<glm::vec3> generateFibonacciSphere(size_t count)
{
    std::vector<glm::vec3> points;

    generateFibonacciSphere(count, points);
    return points;
}

}//namespace voxigen

#endif //_voxigen_coords_h_