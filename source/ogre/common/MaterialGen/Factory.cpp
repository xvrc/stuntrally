#include "Factory.hpp"

#include <stdexcept>

#include "Platform.hpp"
#include "InstanceLoader.hpp"
#include "ShaderSetLoader.hpp"
#include "ShaderSet.hpp"
#include "MaterialInstanceTextureUnit.hpp"

namespace sh
{
	Factory::Factory (Platform* platform)
		: mPlatform(platform)
	{
		mPlatform->setFactory(this);

		// load shader sets
		{
			ShaderSetLoader shaderSetLoader(mPlatform->getBasePath());
			std::map <std::string, ScriptNode*> nodes = shaderSetLoader.getAllConfigScripts();
			for (std::map <std::string, ScriptNode*>::const_iterator it = nodes.begin();
				it != nodes.end(); ++it)
			{
				if (!(it->second->getName() == "shader_set"))
				{
					std::cerr << "sh::Factory: Warning: Unsupported root node type \"" << it->second->getName() << "\" for file type .shaderset" << std::endl;
					break;
				}

				ShaderSet newSet (it->second->findChild("type")->getValue(), it->second->findChild("source")->getValue());

				mShaderSets[it->first] = newSet;
			}
		}

		// load instances
		{
			InstanceLoader instanceLoader(mPlatform->getBasePath());

			std::map <std::string, ScriptNode*> nodes = instanceLoader.getAllConfigScripts();
			for (std::map <std::string, ScriptNode*>::const_iterator it = nodes.begin();
				it != nodes.end(); ++it)
			{
				if (!(it->second->getName() == "material"))
				{
					std::cerr << "sh::Factory: Warning: Unsupported root node type \"" << it->second->getName() << "\" for file type .mat" << std::endl;
					break;
				}

				MaterialInstance newInstance(it->first);
				newInstance._create(mPlatform);

				// first create all passes that are explicitely marked as such
				std::vector<ScriptNode*> passes = it->second->getChildren();
				for (std::vector<ScriptNode*>::const_iterator passIt = passes.begin(); passIt != passes.end(); ++passIt)
				{
					std::string name = (*passIt)->getName();
					if (name != "pass")
						continue;
					std::string val = (*passIt)->getValue();

					MaterialInstancePass* newPass = newInstance.createPass();
					std::vector<ScriptNode*> props = (*passIt)->getChildren();
					for (std::vector<ScriptNode*>::const_iterator propIt = props.begin(); propIt != props.end(); ++propIt)
					{
						std::string name = (*propIt)->getName();
						std::string val = (*propIt)->getValue();

						if (name == "texture_unit")
						{
							MaterialInstanceTextureUnit* newTex = newPass->createTextureUnit(val);
							std::vector<ScriptNode*> texProps = (*propIt)->getChildren();
							for (std::vector<ScriptNode*>::const_iterator texPropIt = texProps.begin(); texPropIt != texProps.end(); ++texPropIt)
							{
								std::string val = (*texPropIt)->getValue();
								newTex->setProperty((*texPropIt)->getName(), makeProperty<StringValue>(new StringValue(val)));
							}
						}
						else
							newPass->setProperty((*propIt)->getName(), makeProperty<StringValue>(new StringValue(val)));
					}
				}

				/// \todo assign all other properties that don't explicitely belong to a pass to the first pass

				std::vector<ScriptNode*> props = it->second->getChildren();
				for (std::vector<ScriptNode*>::const_iterator propIt = props.begin(); propIt != props.end(); ++propIt)
				{
					std::string name = (*propIt)->getName();
					std::string val = (*propIt)->getValue();

					if (name == "parent")
						newInstance._setParentInstance(val);
					else
						newInstance.setProperty((*propIt)->getName(), makeProperty<StringValue>(new StringValue(val)));
				}

				mMaterials[it->first] = newInstance;
			}

			// now that all instances are loaded, replace the parent names with the actual pointers to parent
			for (MaterialMap::iterator it = mMaterials.begin(); it != mMaterials.end(); ++it)
			{
				std::string parent = it->second._getParentInstance();
				if (parent != "")
				{
					if (mMaterials.find (it->second._getParentInstance()) == mMaterials.end())
						throw std::runtime_error ("Unable to find parent for material instance \"" + it->first + "\"");
					it->second.setParent(&mMaterials[parent]);
				}
			}
		}
	}

	Factory::~Factory ()
	{
		delete mPlatform;
	}

	MaterialInstance* Factory::searchInstance (const std::string& name)
	{
		if (mMaterials.find(name) != mMaterials.end())
				return &mMaterials[name];

		return NULL;
	}

	MaterialInstance* Factory::findInstance (const std::string& name)
	{
		assert (mMaterials.find(name) != mMaterials.end());
		return &mMaterials[name];
	}

	MaterialInstance* Factory::requestMaterial (const std::string& name, const std::string& configuration)
	{
		MaterialInstance* m = searchInstance (name);
		if (m)
			m->_createForConfiguration (mPlatform, configuration);
		return m;
	}

	void Factory::notifyFrameEntered ()
	{
		mPlatform->notifyFrameEntered();
	}
}
