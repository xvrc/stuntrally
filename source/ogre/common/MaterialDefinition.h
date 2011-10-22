#ifndef MATERIALDEFINITION_H
#define MATERIALDEFINITION_H

#include <string>

#include <OgreMaterial.h>
#include <OgreStringConverter.h>
#include <OgreString.h>
#include <OgreVector3.h>
#include <OgreVector4.h>
 
class MaterialFactory;

typedef std::map<unsigned int, std::string> textureMap;

enum CullHardwareMode
{
	CULL_HW_NONE=0x00, CULL_HW_CLOCKWISE=0x01, CULL_HW_ANTICLOCKWISE=0x02,

	// or = for depth shadows on
	CULL_HW_CLOCKWISE_OR_NONE=0x03, CULL_HW_ANTICLOCKWISE_OR_NONE=0x04
};

struct MaterialProperties
{
	bool shaders;
	
	// map: tex size, tex name
	textureMap diffuseMaps;
	textureMap alphaMaps; bool transparent;
	textureMap normalMaps; float bumpScale;
	
	CullHardwareMode cullHardware;
	
	
	// reflection
	std::string envMap;
	float reflAmount;
	bool hasFresnel; float fresnelBias, fresnelScale, fresnelPower;
	
	// shadows, lighting
	bool receivesShadows, receivesDepthShadows;
	Ogre::Vector3 ambient; Ogre::Vector4 diffuse; Ogre::Vector4 specular;
	
	float depthBias; bool depthCheck;
	
	//!todo:
	// high priority:
	// alpha_rejection, transparent_sorting
	
	// low priority / future:
	// PPX on/off, shading mode (phong etc) for no shaders,
	// normalmap/shadowmap/envmap "priority",
	// [casts_shadows (+priority) (probably not here)],
	// read terrain lightmap on/off
	// specular map (exponent in diffuse map alpha) [or seperate map for trees]
	// normalHeight (height in normal map alpha) [for parallax mapping]
	
	MaterialProperties(); // constructor with sensible default values
	void setProperty(const std::string& prop, const std::string& value);
};

class MaterialDefinition
{
public:
	MaterialDefinition(MaterialFactory* parent, MaterialProperties* props);
	~MaterialDefinition();
	
	/// get/set
	const std::string& getName() { return mName; };
	MaterialProperties* getProps() { return mProps; };
	void setName(const std::string& name) { mName = name; };
	
	MaterialProperties* mProps;

private:
	MaterialFactory* mParent;
		
	std::string mName; // name of generated material
};

#endif
