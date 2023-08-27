// Originally named tyParticleObjectExt.h

/*Copyright (c) 2021, Tyson Ibele Productions Inc.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this file ("tyParticleObjectExt.h") and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "IParticleObjectExt.h"
#include "mesh.h"
#include <vector>

class Mtl;
struct tyParticleUVWInfo { int channel; UVVert value; };

/*
The tyParticleInterface interface allows you to access
a tyFlow's custom data channels, similar to how
position/rotation/scale/etc values are accessed through 
the regular IParticleObjectExt interface.

USAGE:

tyParticleInterface* tyObj = NULL;

//...acquire interface from baseObject here...

if (tyObj)
{	
	//UpdateTyParticles wraps UpdateParticles. Do not also call 
	//UpdateParticles because it will clear out some data cached 
	//by UpdateTyParticles.
	
	tyObj->UpdateTyParticles(node, t);
	
	//To ensure maximum data access speed, we convert our channel
	//strings into channel indices outside of the particle loop 

	//Channel strings are arbitrary and defined by the user inside
	//the tyFlow's various operators. Safety checks are in place to
	//ensure attempts to access a missing channel will not cause
	//any errors. A default value for missing channels will simply
	//be returned instead (0.0f, Point3::Origin, Matrix3(1))

	//Note: channel names are case-sensitive

	int floatChannel1 = tyObj->FloatChannelToInt(_T("myFloatChannel")); 
	int VectorChannel1 = tyObj->VectorChannelToInt(_T("myVectorChannel1"));
	int VectorChannel2 = tyObj->VectorChannelToInt(_T("myVectorChannel2"));
	int TMChannel1 = tyObj->TMChannelToInt(_T("myTMChannel"));
	
	int numParticles = tyObj->NumParticles();

	for (int q = 0; q < numParticles; q++)
	{
		float f1 = tyObj->GetCustomFloat(q, floatChannel1);
		Point3 v1 = tyObj->GetCustomVector(q, vectorChannel1);
		Point3 v2 = tyObj->GetCustomVector(q, vectorChannel2);
		Matrix3 tm1 = tyObj->GetCustomVector(q, TMChannel1);

		//...etc
	}
}
*/

class tyParticleObjectExt : public IParticleObjectExt
{
public:

	/*
	These functions convert channel strings into channel integers
	*/
	virtual int FloatChannelToInt(TSTR channel) = 0;
	virtual int VectorChannelToInt(TSTR channel) = 0;
	virtual int TMChannelToInt(TSTR channel) = 0;

	/*
	These functions return custom data values for particle 
	indices using channel integers
	*/
	virtual float GetCustomFloat(int index, int channelInt) = 0;
	virtual Point3 GetCustomVector(int index, int channelInt) = 0;
	virtual Matrix3 GetCustomTM(int index, int channelInt) = 0;

	virtual void UpdateTyParticles(INode* node, TimeValue t) = 0;
};

class tyParticleObjectExt2 : public tyParticleObjectExt
{
	public:

	/*
	This is an overload of the default UpdateTyParticles function that
	allows developers to declare which plugin they are making a 
	particle query from, in case there are settings within the flow 
	pertaining to the type of particle data a specific plugin should receive.
	*/	
	virtual void UpdateTyParticles(INode* node, TimeValue t, unsigned int plugin) = 0;

	/*
	The UpdateTyParticles 'plugin' argument should be one of these values
	*/
	enum
	{
		plugin_default = 0,
		plugin_fumefx = 4,
		plugin_phoenixfd = 8,
		plugin_redshift = 12,
		plugin_vray = 16,
		plugin_octane = 20,
		plugin_corona = 24,
		plugin_arnold = 28,
	};

	/*
	This function returns per-particle mesh matID overrides.
	A return value of -1 means no override is set on the particle.
	*/
	virtual int GetParticleMatIDByIndex(int index) = 0;

	/*
	This function returns per-particle material (Mtl*) overrides.
	A return value of NULL means no override is set on the particle and 
	thus the default node material should be used.
	*/
	virtual Mtl* GetParticleMtlByIndex(int index) = 0;

	/*
	This function returns per-particle UVW overrides for specific map 
	channels. 
	The return value is a vector which contains a list of overrides
	and the map channel whose vertices they should be assigned to. An
	empty vector means no UVW overrides have been assigned to the particle.
	*/
	virtual std::vector<tyParticleUVWInfo> GetParticleUVWsByIndex(int index) = 0;

	/*
	These functions return a list of active channel names for 
	each data type
	*/
	virtual std::vector<TSTR> GetFloatChannelNames() = 0;
	virtual std::vector<TSTR> GetVectorChannelNames() = 0;
	virtual std::vector<TSTR> GetTMChannelNames() = 0;	
	
	/*
	This function returns a Mesh that contains any non-particle meshes 
	generated by the flow. It is your responsibility
	to check that the pointer is non-NULL and to delete the pointer 
	when you are done using it. NOTE: this function is currently not
	used in tyFlow/tyCache and will always return NULL.
	*/
	virtual Mesh* GetSecondaryMesh() = 0;
};

class tyParticleObjectExt3 : public tyParticleObjectExt2
{
public:

	/*
	This function returns per-particle simulation group flags
	A return value of 0 means no flags have been set.
	*/
	virtual unsigned int GetParticleSimGroupsByIndex(int index) = 0;

	/*
	This function returns per-particle export group flags
	A return value of 0 means no flags have been set.
	*/
	virtual unsigned int GetParticleExportGroupsByIndex(int index) = 0;

	/*
	This function returns per-particle spin values
	in per-frame units
	*/
	virtual Point3 GetParticleSpinPoint3ByIndex(int index) = 0;

};

class tyParticleObjectExt4 : public tyParticleObjectExt3
{
public:

	/*
	This function returns the map channel where per-vertex 
	velocity data (stored in units/frame) might be found, inside
	any meshes returned by the tyParticleInterface. Note: not 
	all meshes are guaranteed to contain velocity data. It is your 
	duty to check that this map channel is initialized on a given 
	mesh and that its face count is equal to the mesh's face count.
	If both face counts are equal, you can retrieve vertex velocities
	by iterating each mesh face's vertices, and applying the 
	corresponding map face vertex value to the vertex velocity array
	you are constructing. Vertex velocities must be indirectly retrieved 
	by iterating through the faces like this, because even if the map
	vertex count is identical to the mesh vertex count, the map/mesh 
	vertex indices may not correspond to each other. 
	
	Here is an example of how vertex velocities could be retrieved from
	the velocity map channel, through a tyParticleInterface:
	
	////
	
	std::vector<Point3> vertexVelocities(mesh.numVerts, Point3(0,0,0));
	
	int velMapChan = theTyParticleInterface->GetMeshVelocityMapChannel();	
	if (velMapChan >= 0 && mesh.mapSupport(velMapChan))
	{
		MeshMap &map = mesh.maps[velMapChan];
		if (map.fnum == mesh.numFaces)
		{
			for (int f = 0; f < mesh.numFaces; f++)
			{
				Face &meshFace = mesh.faces[f];
				TVFace &mapFace = map.tf[f];
				
				for (int v = 0; v < 3; v++)
				{
					int meshVInx = meshFace.v[v];
					int mapVInx = mapFace.t[v];
					Point3 vel = map.tv[mapVInx];
					vertexVelocities[meshVInx] = vel;
				}
			}
		}
	}
	
	*/
	
	virtual int GetMeshVelocityMapChannel() = 0;
};

struct tyInstance
{
	/*ID contains the unique Birth ID of source particles. This value is
	guaranteed to be unique for each particle in the flow. This value can
	be negative or zero*/
	__int64 ID;

	/*instanceID contains the arbitrary, user-defined instance ID of source 
	particles. Texmaps can make use of this value at rendertime. This value
	can be negative or zero*/
	__int64 instanceID;

	/*tm0 and tm1 contain the instance's transform at the start and end of the 
	motion blur interval (specified by the arguments passed to CollectInstances).
	Instance velocity/spin over the motion blur interval should be derived from
	these values*/
	Matrix3 tm0;
	Matrix3 tm1;
		
	/*mappingOverrides contains mapping override data for channels specified
	in the tyParticleUVWInfo struct. Each value should override all mapping
	vertex values of the instance mesh for the specified mapping channel*/
	std::vector<tyParticleUVWInfo> mappingOverrides;

	/*materialOverride contains the material override for the instance. A value
	of NULL means no override should be applied*/
	Mtl* materialOverride;

	/*matIDOverride contains the material ID override for the instance. A value
	of -1 means no override should be applied*/
	int matIDOverride;

	/*vel is the per-frame particle velocity of the instance. Note: this value
	is stored for completeness, but should not be used by developers to calculate 
	motion blur. Motion blur should be calculated using tm0 and tm1 instead.
	*/
	Point3 vel;

	/*spin is the per-frame particle spin of the instance. Note: this value
	is stored for completeness, but should not be used by developers to calculate
	motion blur. Motion blur should be calculated using tm0 and tm1 instead.
	*/
	Point3 spin;
};

struct tyInstanceInfo
{
	/*mesh contains the Mesh pointer for this instance group*/
	Mesh *mesh;

	/*meshVelocityMapChannel defines which map channel of the mesh
	contains per-vertex velocity data (stored in units/frame). A value
	of -1 means the mesh contains no per-vertex velocity data.*/
	int meshVelocityMapChannel;

	/*instances is an array of tyInstances that all share the same mesh 
	pointer (defined above). It also contains any overrides which should 
	be applied on a per-instance basis.*/
	std::vector<tyInstance> instances;
};

struct tyInstanceNodeInfo
{
	/*node contains the INode pointer for this instanceNode group*/
	INode* node;

	/*instances is an array of tyInstances that all share the same node
	pointer (defined above). It also contains any overrides which should
	be applied on a per-node basis. Depending on the node type, not all
	overrides may be applicable (ex: a light node cannot take a material
	ID override), but they are all included just in case they're needed.*/
	std::vector<tyInstance> instances;
};

class tyParticleObjectExt5 : public tyParticleObjectExt4
{
public:

	/*
	This helper function collects instances (particles that	share 
	the same mesh pointer) and groups them together, along with 
	any per-particle property overrides. It is a quick way to 
	collect all particle instances for rendering. The arguments 
	'moblurStart' and 'moblurEnd' should be the start and end of the 
	desired motion blur interval, for proper particle transform retrieval. 
	Note: this function calls UpdateTyParticles internally for all 
	time values, so UpdateTyParticles does not need to be manually 
	called before calls	to CollectInstances. 
	*/
	virtual std::vector<tyInstanceInfo> CollectInstances(
		INode* node, 
		TimeValue moblurStart, 
		TimeValue moblurEnd, 
		unsigned int plugin) = 0;

	/*
	This function returns per-particle instance ID. This is a user-defined
	ID that can be arbitrary and independent from each particle's birth ID.
	*/
	virtual int GetParticleInstanceIDByIndex(int index) = 0;
};

class tyParticleObjectExt6 : public tyParticleObjectExt5
{
public:

	/*
	This helper function collects instanceNodes (nodes scattered with
	particles at rendertime) and groups them together, along with
	any per-particle property overrides. It is a quick way to
	collect all particle instanceNodes for rendering. The arguments
	'moblurStart' and 'moblurEnd' should be the start and end of the
	desired motion blur interval, for proper particle transform retrieval.
	Note: this function calls UpdateTyParticles internally for all
	time values, so UpdateTyParticles does not need to be manually
	called before calls	to CollectInstanceNodes.
	*/
	virtual std::vector<tyInstanceNodeInfo> CollectInstanceNodes(
		INode* node,
		TimeValue moblurStart,
		TimeValue moblurEnd,
		unsigned int plugin) = 0;

	/*
	This function returns per-particle instanceNode. This is a user-defined
	render-only node which corresponds to each particle. NULL means no 
	node has been assigned.
	*/
	virtual INode* GetParticleInstanceNodeByIndex(int index) = 0;
};

class tyParticleObjectExt7 : public tyParticleObjectExt6
{
public:

	/*
	This function returns per-particle mass values.
	*/
	virtual float GetParticleMassByIndex(int index) = 0;

};


typedef tyParticleObjectExt7 tyParticleInterface;

#define TYPARTICLE_INTERFACE Interface_ID(0x1213b15, 0x1e23501)
#define GetTyParticleInterface(obj) ((tyParticleInterface*)obj->GetInterface(TYPARTICLE_INTERFACE))

/*
Helper interface to force the retrieval of a regular tyParticleInterface interface,
on tyFlow/tyCache objects, even if their "particle interface" option is disabled.
*/
#define TYPARTICLE_INTERFACE_FORCED Interface_ID(0x1213b15, 0x1e23503)
#define GetTyParticleInterfaceForced(obj) (tyParticleInterface*)obj->GetInterface(TYPARTICLE_INTERFACE_FORCED)

/*
Helper interface to force the retrieval of a regular IParticleObjectExt interface,
on tyFlow/tyCache objects, even if their "particle interface" option is disabled.
*/
#define PARTICLEOBJECTEXT_INTERFACE_FORCED Interface_ID(0x1213b15, 0x1e23502)
#define GetParticleInterfaceForced(obj) (IParticleObjectExt*)obj->GetInterface(PARTICLEOBJECTEXT_INTERFACE_FORCED)