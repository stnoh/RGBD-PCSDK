/*******************************************************************************
The original source code is "hsklu.h" in Intel Hand Skeletal Library.
https://software.intel.com/protected-download/393460/393178

Modified by Seung-Tak Noh (seungtak.noh [at] gmail.com), 2013-2015
*******************************************************************************/
#pragma once

#ifndef HAND_SKELETON_UTILITY
#define HAND_SKELETON_UTILITY

#include "common.h"
#include "hskl.h"

#include <cmath>


namespace hskl
{
	// Basic linear algebra types and operations
	struct float3 { float x,y,z;   float3() : x(),y(),z()     {} float3(float x, float y, float z         ) : x(x),y(y),z(z)      {} operator const float * () const { return &x; } };
	struct float4 { float x,y,z,w; float4() : x(),y(),z(),w() {} float4(float x, float y, float z, float w) : x(x),y(y),z(z),w(w) {} operator const float * () const { return &x; } };
	struct float4x4 { float4 x,y,z,w; operator const float * () const { return x; } };
	inline float3 operator - (const float3 & v)                   { return float3(-v.x,-v.y,-v.z); }
	inline float3 operator + (const float3 & a, const float3 & b) { return float3(a.x+b.x,a.y+b.y,a.z+b.z); }
	inline float3 operator - (const float3 & a, const float3 & b) { return float3(a.x-b.x,a.y-b.y,a.z-b.z); }
	inline float3 operator * (const float3 & a, float b)          { return float3(a.x*b  ,a.y*b  ,a.z*b  ); }
	inline float3 cross      (const float3 & a, const float3 & b) { return float3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x); }
	inline float  dot        (const float3 & a, const float3 & b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
	inline float4 qinv       (const float4 & q)                   { return float4(-q.x,-q.y,-q.z,q.w); }
	inline float4 qmul       (const float4 & a, const float4 & b) { return float4(a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y, 
																				  a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,
																				  a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w,
																				  a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z); }
	inline float3 qxdir      (const float4 & q)                   { return float3(1-2*(q.y*q.y+q.z*q.z), 2*(q.x*q.y+q.w*q.z), 2*(q.x*q.z-q.w*q.y)); }
	inline float3 qydir      (const float4 & q)                   { return float3(2*(q.x*q.y-q.w*q.z), 1-2*(q.x*q.x+q.z*q.z), 2*(q.y*q.z+q.w*q.x)); }
	inline float3 qzdir      (const float4 & q)                   { return float3(2*(q.x*q.z+q.w*q.y), 2*(q.y*q.z-q.w*q.x), 1-2*(q.x*q.x+q.y*q.y)); }
	inline float3 qtransform (const float4 & q, const float3 & p) { return qxdir(q)*p.x + qydir(q)*p.y + qzdir(q)*p.z; }


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Tracking system that encapsulates a handle to tracking engine and a depth sensor, and wires them together
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class Tracker
	{
		// tracking core module
		hskl_tracker tracker;
		
		// Disallow copy-construction & assignment
		Tracker(const Tracker &);
		Tracker &operator = (const Tracker &);

	public:
		Tracker() : tracker() {}
		~Tracker() { if(tracker) hsklDestroyTracker(tracker); }

		// wrapping
		void									SetModelType(hskl_model model)						{ hsklSetModelType(tracker,model); }
		void									SetHandMeasurements(float width, float height)		{ hsklSetHandMeasurements(tracker,width,height); }
		
		float3									GetPosition(int bone) const							{ return (const float3 &)hsklGetPosition(tracker,bone); }
		float4									GetOrientation(int bone) const						{ return (const float4 &)hsklGetOrientation(tracker,bone); }
		float									GetTrackingError(int bone) const					{ return hsklGetTrackingError(tracker,bone); }

		int										GetBoneCount() const								{ return hsklGetBoneCount(tracker); }     
		int										GetVertexCount(int bone) const						{ return hsklGetVertexCount(tracker,bone); }
		const float3 *							GetVertices(int bone) const							{ return (const float3 *)hsklGetVertices(tracker,bone); }
		int										GetTriangleCount(int bone) const					{ return hsklGetTriangleCount(tracker,bone); }
		const hskl_int3 *						GetTriangles(int bone) const						{ return hsklGetTriangles(tracker,bone); }
		float3									GetCenterOfMass(int bone) const						{ return (const float3 &)hsklGetCenterOfMass(tracker,bone); }
		int										GetParentBone(int bone) const						{ return hsklGetParentBone(tracker,bone); }
		float3									GetAnchorPoint(int bone) const						{ return (const float3 &)hsklGetAnchorPoint(tracker,bone); }
		
		const unsigned char *					GetSegmentationMask() const							{ return hsklGetSegmentationMask(tracker); }

		// declarations
		float4x4								GetBoneModelMatrix(int bone) const;					// Obtain a 4x4 transformation matrix representing the pose of the bone
		float3									GetBoneTip(int bone) const;							// Obtain the position of the tip of a bone, such as a fingertip

		// modified by sNoh
		bool									Init(float fx, float fy, int dimx, int dimy);
		void									Update(unsigned short* depth, unsigned short* conf);
	};

	///////////////////////////
	// Math/geometry helpers //
	///////////////////////////
	inline float4x4 Tracker::GetBoneModelMatrix(int bone) const
	{
		const auto q = GetOrientation(bone);
		float4x4 m;
		(float3&)m.x = qxdir(q);
		(float3&)m.y = qydir(q);
		(float3&)m.z = qzdir(q);
		(float3&)m.w = GetPosition(bone); 
		m.w.w = 1;
		return m;
	}

	inline float3 Tracker::GetBoneTip(int bone) const 
	{
		float maxZ = 0; auto vertices = GetVertices(bone);
		for(int i=0; i<GetVertexCount(bone); ++i) if(vertices[i].z > maxZ) maxZ = vertices[i].z;
		return GetPosition(bone) + qzdir(GetOrientation(bone)) * maxZ;
	}

	
	//////////////////////
	// added by sNoh
	//////////////////////
	inline float3 mult(hskl::float4x4 mat, hskl::float3 _vertex)
	{
		hskl::float3 vertex;

		vertex.x = mat.x.x * _vertex.x + mat.y.x * _vertex.y + mat.z.x * _vertex.z + mat.w.x * 1.0f;
		vertex.y = mat.x.y * _vertex.x + mat.y.y * _vertex.y + mat.z.y * _vertex.z + mat.w.y * 1.0f;
		vertex.z = mat.x.z * _vertex.x + mat.y.z * _vertex.y + mat.z.z * _vertex.z + mat.w.z * 1.0f;

		return vertex;
	}

	inline float4 mult(hskl::float4x4 mat, hskl::float4 _vertex)
	{
		hskl::float4 vertex;

		vertex.x = mat.x.x * _vertex.x + mat.y.x * _vertex.y + mat.z.x * _vertex.z + mat.w.x * _vertex.w;
		vertex.y = mat.x.y * _vertex.x + mat.y.y * _vertex.y + mat.z.y * _vertex.z + mat.w.y * _vertex.w;
		vertex.z = mat.x.z * _vertex.x + mat.y.z * _vertex.y + mat.z.z * _vertex.z + mat.w.z * _vertex.w;
		vertex.w = mat.x.x * _vertex.x + mat.y.x * _vertex.y + mat.z.x * _vertex.z + mat.w.x * _vertex.w;

		return vertex;
	}

	inline float4x4 inv(hskl::float4x4 mat)
	{
		hskl::float4x4 mat_inv;

		mat_inv.x = float4(mat.x.x, mat.y.x, mat.z.x, 0);
		mat_inv.y = float4(mat.x.y, mat.y.y, mat.z.y, 0);
		mat_inv.z = float4(mat.x.z, mat.y.z, mat.z.z, 0);
		mat_inv.w = float4(0,0,0,0);

		float4 p_inv = mult(mat_inv, mat.w);
		mat_inv.w = float4(-p_inv.x, -p_inv.y, -p_inv.z, 1);

		return mat_inv;
	}


	//////////////////////
	// modified by sNoh
	//////////////////////
	inline bool Tracker::Init(float fx, float fy, int dimx, int dimy)
	{
		// calculate fov_x and fox_y for setting
		float fovx = atan(dimx / (fx*2))*2, fovy = atan(dimy / (fy*2))*2;

		// Initialize tracking library
		tracker = hsklCreateTracker(HSKL_COORDS_X_RIGHT_Y_DOWN_Z_FWD, HSKL_API_VERSION); // right-handed coordinate: OpenGL
		hsklSetSensorProperties(tracker, HSKL_SENSOR_CREATIVE, dimx, dimy, fovx, fovy);

		// Set hand model
		SetModelType(HSKL_MODEL_RIGHT_HAND); // track both hands
		SetHandMeasurements(0.08f, 0.16f); // hand size in meter
		return true;
	}

	inline void Tracker::Update(unsigned short* raw_depth, unsigned short* raw_confidence)
	{
		hsklTrackOneFrame(tracker, raw_depth, raw_confidence);
	}
}

#endif
