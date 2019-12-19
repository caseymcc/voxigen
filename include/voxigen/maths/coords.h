#ifndef _voxigen_coords_h_
#define _voxigen_coords_h_

#include "voxigen/voxigen_export.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <math.h>
#include <vector>

namespace voxigen
{

enum Projections
{
    Cartesian,
    Spherical,
    Equirectangular,
    Stereographic
};

template<Projections _Type>
struct ProjectionDetails
{
//	using PointType=float;
};

template<>
struct ProjectionDetails<Projections::Cartesian>
{
    using PointType=glm::vec3;
};

template<>
struct ProjectionDetails<Projections::Spherical>
{
    using PointType=glm::vec3;
};

template<>
struct ProjectionDetails<Projections::Equirectangular>
{
    using PointType=glm::vec2;
};

template<>
struct ProjectionDetails<Projections::Stereographic>
{
    using PointType=glm::vec2;
};

template<Projections _Projection>
inline float distance2(const typename ProjectionDetails<_Projection>::PointType &pt1, const typename ProjectionDetails<_Projection>::PointType &pt2)
{
    return glm::distance2(pt1, pt2);
}

template<Projections _From, Projections _To>
void projectPoint(const typename ProjectionDetails<_From>::PointType &inPoint, typename ProjectionDetails<_To>::PointType &outPoint)
{
    static_assert(false);
}

template<>
inline void projectPoint<Projections::Cartesian, Projections::Spherical>(const ProjectionDetails<Projections::Cartesian>::PointType &inPoint, ProjectionDetails<Projections::Spherical>::PointType &outPoint)
{
    float d=sqrt((inPoint.x*inPoint.x)+(inPoint.y*inPoint.y));

    outPoint.x=sqrt((d*d)+(inPoint.z*inPoint.z)); //radius
    outPoint.y=atan2(inPoint.y, inPoint.x); //azimuth, theta
    if(outPoint.y<0.0f)
        outPoint.y+=glm::two_pi<float>();
    outPoint.z=atan2(inPoint.z, d); //inclination, phi
}

template<>
inline void projectPoint<Projections::Spherical, Projections::Cartesian>(const ProjectionDetails<Projections::Spherical>::PointType &inPoint, ProjectionDetails<Projections::Cartesian>::PointType &outPoint)
{
    float d=inPoint.x*cos(inPoint.z);

    outPoint.x=d*cos(inPoint.y);
    outPoint.y=d*sin(inPoint.y);
    outPoint.z=inPoint.x*sin(inPoint.z);
}

template<>
inline void projectPoint<Projections::Spherical, Projections::Equirectangular>(const ProjectionDetails<Projections::Spherical>::PointType &inPoint, ProjectionDetails<Projections::Equirectangular>::PointType &outPoint)
{
    outPoint.x=inPoint.y/glm::two_pi<float>();
    outPoint.y=(glm::half_pi<float>()-inPoint.z)/glm::pi<float>();
}

template<>
inline void projectPoint<Projections::Equirectangular, Projections::Spherical>(const ProjectionDetails<Projections::Equirectangular>::PointType &inPoint, ProjectionDetails<Projections::Spherical>::PointType &outPoint)
{
    outPoint.x=1.0f;
    outPoint.y=glm::two_pi<float>()*(inPoint.x);
    outPoint.z=glm::half_pi<float>()-(glm::pi<float>()*(inPoint.y));
}

template<>
inline void projectPoint<Projections::Equirectangular, Projections::Cartesian>(const ProjectionDetails<Projections::Equirectangular>::PointType &inPoint, ProjectionDetails<Projections::Cartesian>::PointType &outPoint)
{
    typename ProjectionDetails<Projections::Spherical>::PointType sphericalPoint;

    projectPoint<Projections::Equirectangular, Projections::Spherical>(inPoint, sphericalPoint);
    projectPoint<Projections::Spherical, Projections::Cartesian>(sphericalPoint, outPoint);
}

template<>
inline void projectPoint<Projections::Cartesian, Projections::Equirectangular>(const ProjectionDetails<Projections::Cartesian>::PointType &inPoint, ProjectionDetails<Projections::Equirectangular>::PointType &outPoint)
{
    typename ProjectionDetails<Projections::Spherical>::PointType sphericalPoint;

    projectPoint<Projections::Cartesian, Projections::Spherical>(inPoint, sphericalPoint);
    projectPoint<Projections::Spherical, Projections::Equirectangular>(sphericalPoint, outPoint);
}

template<>
inline void projectPoint<Projections::Cartesian, Projections::Stereographic>(const ProjectionDetails<Projections::Cartesian>::PointType &inPoint, ProjectionDetails<Projections::Stereographic>::PointType &outPoint)
{
    float d=(1.0f-inPoint.z);

    outPoint.x=inPoint.x/d;
    outPoint.y=inPoint.y/d;
}

template<>
inline void projectPoint<Projections::Stereographic, Projections::Cartesian>(const ProjectionDetails<Projections::Stereographic>::PointType &inPoint, ProjectionDetails<Projections::Cartesian>::PointType &outPoint)
{
    float d=(1.0f+inPoint.x*inPoint.x+inPoint.y*inPoint.y);
    
    outPoint.x=(2.0f*inPoint.x)/d;
    outPoint.y=(2.0f*inPoint.y)/d;
    outPoint.z=(-2.0f+d)/d;
}

template<Projections _Projection>
float perpendicularDistanceSqrd(const typename ProjectionDetails<_Projection>::PointType &point, const typename ProjectionDetails<_Projection>::PointType &lineStart,
    const typename ProjectionDetails<_Projection>::PointType &lineEnd)
{
    using PointType=ProjectionDetails<_Projection>::PointType;

    PointType delta=lineEnd-lineStart;
    PointType pointDelta=point-lineStart;

    delta=glm::normalize(delta);

    float dot=glm::dot(delta, pointDelta);

    delta=pointDelta-(delta*dot);

    return glm::length(delta);
}

template<>
inline float perpendicularDistanceSqrd<Projections::Spherical>(const typename ProjectionDetails<Projections::Spherical>::PointType &point, const typename ProjectionDetails<Projections::Spherical>::PointType &lineStart,
	const typename ProjectionDetails<Projections::Spherical>::PointType &lineEnd)
{
	using CartPointType=ProjectionDetails<Projections::Cartesian>::PointType;

	CartPointType cartPoint;
	CartPointType cartLineStart;
	CartPointType cartLineEnd;
	
	projectPoint<Projections::Spherical, Projections::Cartesian>(point, cartPoint);
	projectPoint<Projections::Spherical, Projections::Cartesian>(lineStart, cartLineStart);
	projectPoint<Projections::Spherical, Projections::Cartesian>(lineEnd, cartLineEnd);

	return perpendicularDistanceSqrd<Projections::Cartesian>(cartPoint, cartLineStart, cartLineEnd);
}

inline void projectVector(const typename ProjectionDetails<Projections::Cartesian>::PointType &vec, const typename ProjectionDetails<Projections::Spherical>::PointType &sphericalPoint, typename ProjectionDetails<Projections::Spherical>::PointType &outVec)
{
//    ProjectionDetails<Projections::Spherical>::PointType sphericalPoint;
//
//    projectPoint<Projections::Cartesian, Projections::Spherical>(point, sphericalPoint);

//    glm::mat3 mat;

    float cosTheta=cos(sphericalPoint.y);
    float sinTheta=sin(sphericalPoint.y);
    float cosPhi=cos(sphericalPoint.z);
    float sinPhi=sin(sphericalPoint.z);

    if(cosPhi == 0.0f)
    {//handle poles
        outVec.x=cosTheta*cosPhi*vec.x+sinTheta*cosPhi*vec.y+sinPhi*vec.z;
        outVec.y=0.0f;
        outVec.z=-cosTheta*vec.x-sinTheta*vec.y;
    }
    else
    {
        outVec.x=cosTheta*cosPhi*vec.x+sinTheta*cosPhi*vec.y+sinPhi*vec.z;
        outVec.y=-sinTheta*vec.x+cosTheta*vec.y;
        outVec.z=-cosTheta*sinPhi*vec.x-sinTheta*sinPhi*vec.y+cosPhi*vec.z;
    }
}

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
        projectPoint<Projections::Equirectangular, Projections::Spherical>(inPoints[i], outPoints[i]);
//        const glm::vec2 &inPoint=inPoints[i];
//        glm::vec3 &outPoint=outPoints[i];
//        
//        float theta=glm::two_pi<float>()*(inPoint.x);
//        float phi=glm::half_pi<float>()-(glm::pi<float>()*(inPoint.y));
//        float d=radius*cos(phi);
//
//        outPoint.x=d*cos(theta);
//        outPoint.y=d*sin(theta);
//        outPoint.z=sin(phi);//assuming unit sphere 1.0f*
    }
}

inline void cartesianToSpherical(const std::vector<glm::vec3> &inPoints, std::vector<glm::vec3> &outPoints)
{
    if(outPoints.size()!=inPoints.size())
        outPoints.resize(inPoints.size());

    for(size_t i=0; i<inPoints.size(); ++i)
    {
        projectPoint<Projections::Cartesian, Projections::Spherical>(inPoints[i], outPoints[i]);
//        const glm::vec3 &inPoint=inPoints[i];
//        glm::vec3 &outPoint=outPoints[i];
//
//        float d=sqrt((inPoint.x*inPoint.x)+(inPoint.y*inPoint.y));
//        
//        outPoint.x=sqrt((d*d)+(inPoint.z*inPoint.z)); //radius
//        outPoint.y=acos(inPoint.x/d); //azimuth, theta
//        outPoint.z=atan2(inPoint.z, d); //inclination, phi
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
        projectPoint<Projections::Spherical, Projections::Cartesian>(inPoints[i], outPoints[i]);
//        const glm::vec3 &inPoint=inPoints[i];
//        glm::vec3 &outPoint=outPoints[i];
//
//        float d=inPoint.x*cos(inPoint.z);
//
//        outPoint.x=d*cos(inPoint.y);
//        outPoint.y=d*sin(inPoint.y);
//        outPoint.z=inPoint.x*sin(inPoint.z);
    }
}

inline std::vector<glm::vec3> sphericalToCartesian(const std::vector<glm::vec3> &inPoints)
{
    std::vector<glm::vec3> outPoints;

    sphericalToCartesian(inPoints, outPoints);
    return outPoints;
}

inline void cartesianToStereographic(const std::vector<glm::vec3> &inPoints, std::vector<glm::vec2> &outPoints)
{
    if(outPoints.size()!=inPoints.size())
        outPoints.resize(inPoints.size());

    for(int i=0; i<inPoints.size(); ++i)
    {
        projectPoint<Projections::Cartesian, Projections::Stereographic>(inPoints[i], outPoints[i]);
//        const glm::vec3 &inPoint=inPoints[i];
//        glm::vec2 &outPoint=outPoints[i];
//        float d=(1.0f-inPoint.z);
//
//        outPoint.x=inPoint.x/d;
//        outPoint.y=inPoint.y/d;
    }
}

inline std::vector<glm::vec2> cartesianToStereographic(const std::vector<glm::vec3> &inPoints)
{
    std::vector<glm::vec2> outPoints;

    cartesianToStereographic(inPoints, outPoints);
    return outPoints;
}

inline void stereographicToCartesian(const std::vector<glm::vec2> &inPoints, std::vector<glm::vec3> &outPoints)
{
    if(outPoints.size()!=inPoints.size())
        outPoints.resize(inPoints.size());

    for(int i=0; i<inPoints.size(); ++i)
    {
        projectPoint<Projections::Stereographic, Projections::Cartesian>(inPoints[i], outPoints[i]);
//        const glm::vec2 &inPoint=inPoints[i];
//        glm::vec3 &outPoint=outPoints[i];
//        
//        float d=(1.0f+inPoint.x*inPoint.x+inPoint.y*inPoint.y);
//
//        outPoint.x=(2.0f*inPoint.x)/d;
//        outPoint.y=(2.0f*inPoint.y)/d;
//        outPoint.z=(-2.0f+d)/d;
    }
}

inline std::vector<glm::vec3> stereographicToCartesian(const std::vector<glm::vec2> &inPoints)
{
    std::vector<glm::vec3> outPoints;

    stereographicToCartesian(inPoints, outPoints);
    return outPoints;
}

//equirectangular returned as points 0-1.0f from top left corner of image
inline void sphericalToEquirectangular(const std::vector<glm::vec3> &inPoints, std::vector<glm::vec2> &outPoints)
{
    if(outPoints.size()!=inPoints.size())
        outPoints.resize(inPoints.size());

    for(int i=0; i<inPoints.size(); ++i)
    {
        projectPoint<Projections::Spherical, Projections::Equirectangular>(inPoints[i], outPoints[i]);
//        const glm::vec3 &inPoint=inPoints[i];
//        glm::vec2 &outPoint=outPoints[i];
//
//        outPoint.x=inPoint.y/glm::two_pi<float>();
//        outPoint.y=(glm::half_pi<float>()-inPoint.z)/glm::pi<float>();
    }
}

inline std::vector<glm::vec2> sphericalToEquirectangular(const std::vector<glm::vec3> &inPoints)
{
    std::vector<glm::vec2> outPoints;

    sphericalToEquirectangular(inPoints, outPoints);
    return outPoints;
}

template<typename _Type>
_Type wrap(_Type value, _Type min, _Type max, _Type delta)
{
    if(value<min)
        return value+delta;
    if(value>max)
        return value-delta;
    return value;
}

//Generates a unit sphere using the golden section so that nodes in nearby bands are not too close, return format is in sphereical coordinates (radius, azimuth, inclination) or (r, theta, phi)
//jitter is vector of random numbers between -1.0<->1.0
inline void generateFibonacciSphere(size_t count, std::vector<glm::vec3> &points, const std::vector<float> &jitter)
{
    if(points.size() != count)
        points.resize(count);

    float longitudeDelta=glm::pi<float>()*(3.0f-sqrt(5.0f));  //~2.39996323 

    float dz=2.0f/count;
    float z=1.0f-(dz/2.0f);

    float longitude=0.0f;

    for(size_t i=0; i<count; i++)
    {
        glm::vec3 &point=points[i];

        float d=sqrt(1.0f-z*z);

        point.x=1.0f; //radius
        point.y=wrap(longitude+jitter[i]*longitudeDelta, 0.0f, glm::two_pi<float>(), glm::two_pi<float>()); //azimuth, theta
        point.z=wrap(asin(z)+jitter[i]*dz, -glm::half_pi<float>(), glm::half_pi<float>(), glm::pi<float>()); //inclination, phi

        z-=dz;
        longitude+=longitudeDelta;

        longitude=wrap(longitude, 0.0f, glm::two_pi<float>(), glm::two_pi<float>());
    }
}

inline std::vector<glm::vec3> generateFibonacciSphere(size_t count, const std::vector<float> &jitter)
{
    std::vector<glm::vec3> points;

    generateFibonacciSphere(count, points, jitter);
    return points;
}

//projects polyline from one coordinate system to another, will up-sample line if needed
template<Projections _From, Projections _To>
inline void projectPolyLine(const std::vector<typename ProjectionDetails<_From>::PointType> &inPoints, std::vector<typename ProjectionDetails<_To>::PointType> &outPoints, float distanceThreshold=2.0f)
{
	if(inPoints.size() < 2)
		return;

	using FromPointType=typename ProjectionDetails<_From>::PointType;
	using ToPointType=typename ProjectionDetails<_To>::PointType;

	std::vector<FromPointType> nextFromPoint;
	std::vector<ToPointType> nextToPoint;

	nextFromPoint.resize(inPoints.size()-1);
	nextToPoint.resize(inPoints.size()-1);

	if(outPoints.capacity()!=inPoints.size())
		outPoints.reserve(inPoints.size());
	
	for(size_t i=inPoints.size()-1; i>0; --i)
	{
		nextFromPoint[i-1]=inPoints[i];
		projectPoint<_From, _To>(inPoints[i], nextToPoint[i-1]);
	}

	outPoints.resize(1);
	
	FromPointType inPoint=inPoints[0];
	projectPoint<_From, _To>(inPoints[0], outPoints[0]);

	while(!nextToPoint.empty())
	{
		ToPointType &pt1=outPoints.back();
		ToPointType &pt2=nextToPoint.back();
		
        //if points are close skip up-sample
        float toDistance=distance2<_To>(pt1, pt2);
        if(toDistance <= (4*distanceThreshold))
        {
            inPoint=nextFromPoint.back();
            nextFromPoint.pop_back();
            outPoints.push_back(nextToPoint.back());
            nextToPoint.pop_back();
            continue;
        }

		FromPointType &fromPt2=nextFromPoint.back();
		FromPointType fromMidpoint=(inPoint+fromPt2)/2.0f;
		ToPointType midpoint;

		//get midpoint in original coordinates and project to new coordinates
		projectPoint<_From, _To>(fromMidpoint, midpoint);

		float distance=perpendicularDistanceSqrd<_To>(midpoint, pt1, pt2);

		//check distance from point to line in new coordinates, if greater than threshold keep point
		if(distance > distanceThreshold)
		{
			nextFromPoint.push_back(fromMidpoint);
			nextToPoint.push_back(midpoint);
		}
		else
		{
			inPoint=nextFromPoint.back();
			nextFromPoint.pop_back();
			outPoints.push_back(nextToPoint.back());
			nextToPoint.pop_back();
		}
	}
}


}//namespace voxigen

#endif //_voxigen_coords_h_