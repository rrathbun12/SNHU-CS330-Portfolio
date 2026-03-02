///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

 /***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	/*** STUDENTS - add the code BELOW for loading the textures that ***/
	/*** will be used for mapping to objects in the 3D scene. Up to  ***/
	/*** 16 textures can be loaded per scene. Refer to the code in   ***/
	/*** the OpenGL Sample for help.                                 ***/

	bool bReturn = false;

	bReturn = CreateGLTexture(
		"textures/asphalt.jpg",
		"road");
	bReturn = CreateGLTexture(
		"textures/brick.jpg",
		"brick");
	bReturn = CreateGLTexture(
		"textures/carbonfiber.jpg",
		"rim");
	bReturn = CreateGLTexture(
		"textures/rubber.png",
		"tire");
	bReturn = CreateGLTexture(
		"textures/aluminium.jpg",
		"hub");
	bReturn = CreateGLTexture(
		"textures/silver.jpg",
		"spoke");
	bReturn = CreateGLTexture(
		"textures/barrier.png",
		"barrier");
	bReturn = CreateGLTexture(
		"textures/barrier2.png",
		"barrier2");
	bReturn = CreateGLTexture(
		"textures/banner.jpg",
		"banner");
	bReturn = CreateGLTexture(
		"textures/frame.png",
		"frame");
	
	bReturn = CreateGLTexture(
		"textures/whitewood.png",
		"whitewood");

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}
	/***********************************************************
	 *  DefineObjectMaterials()
	 *
	 *  This method is used for configuring the various material
	 *  settings for all of the objects within the 3D scene.
	 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{

	// Road - asphalt
	OBJECT_MATERIAL roadMaterial;
	roadMaterial.diffuseColor = glm::vec3(0.30f, 0.30f, 0.30f);
	roadMaterial.specularColor = glm::vec3(0.05f, 0.05f, 0.05f);
	roadMaterial.shininess = 2.0;
	roadMaterial.tag = "roadMaterial";
	m_objectMaterials.push_back(roadMaterial);

	// Brick - for houses in background
	OBJECT_MATERIAL brickMaterial;
	brickMaterial.diffuseColor = glm::vec3(0.45f, 0.12f, 0.10f);   // dark red brick color to tint the texture darker
	brickMaterial.specularColor = glm::vec3(0.03f, 0.03f, 0.03f);  
	brickMaterial.shininess = 5.0f;                               
	brickMaterial.tag = "brickMaterial";
	m_objectMaterials.push_back(brickMaterial);

	// Rim - carbon fiber 
	OBJECT_MATERIAL rimMaterial;
	rimMaterial.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	rimMaterial.specularColor = glm::vec3(0.35f, 0.35f, 0.35f);
	rimMaterial.shininess = 24.0;
	rimMaterial.tag = "rimMaterial";
	m_objectMaterials.push_back(rimMaterial);

	// Tire - rubber
	OBJECT_MATERIAL tireMaterial;
	tireMaterial.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	tireMaterial.specularColor = glm::vec3(0.02f, 0.02f, 0.02f);
	tireMaterial.shininess = 1.0;
	tireMaterial.tag = "tireMaterial";
	m_objectMaterials.push_back(tireMaterial);

	// Hub - aluminium
	OBJECT_MATERIAL hubMaterial;
	hubMaterial.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	hubMaterial.specularColor = glm::vec3(0.55f, 0.55f, 0.60f);
	hubMaterial.shininess = 16.0;
	hubMaterial.tag = "hubMaterial";
	m_objectMaterials.push_back(hubMaterial);

	// Spoke - steel/silver
	OBJECT_MATERIAL spokeMaterial;
	spokeMaterial.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	spokeMaterial.specularColor = glm::vec3(0.80f, 0.80f, 0.85f);
	spokeMaterial.shininess = 48.0;
	spokeMaterial.tag = "spokeMaterial";
	m_objectMaterials.push_back(spokeMaterial);

	// Vinyl - satin
	OBJECT_MATERIAL vinylMaterial;
	vinylMaterial.diffuseColor = glm::vec3(0.85f, 0.85f, 0.86f);
	vinylMaterial.specularColor = glm::vec3(0.35f, 0.35f, 0.38f);
	vinylMaterial.shininess = 32.0;
	vinylMaterial.tag = "vinylMaterial";
	m_objectMaterials.push_back(vinylMaterial);

	// Barrier Plastic 
	OBJECT_MATERIAL barrierPlasticMaterial;
	barrierPlasticMaterial.diffuseColor = glm::vec3(0.90f, 0.90f, 0.90f);
	barrierPlasticMaterial.specularColor = glm::vec3(0.10f, 0.10f, 0.10f);
	barrierPlasticMaterial.shininess = 4.0;
	barrierPlasticMaterial.tag = "barrierPlastic";
	m_objectMaterials.push_back(barrierPlasticMaterial);

	// Dark Plastic - for arch
	OBJECT_MATERIAL darkPlasticMaterial;
	darkPlasticMaterial.diffuseColor = glm::vec3(0.35f, 0.35f, 0.35f);
	darkPlasticMaterial.specularColor = glm::vec3(0.25f, 0.25f, 0.25f);
	darkPlasticMaterial.shininess = 24.0f;
	darkPlasticMaterial.tag = "darkPlasticMaterial";
	m_objectMaterials.push_back(darkPlasticMaterial);

	// Frame - Glossy White Finish
	OBJECT_MATERIAL frameMaterial;
	frameMaterial.diffuseColor = glm::vec3(0.95f, 0.95f, 0.96f);
	frameMaterial.specularColor = glm::vec3(0.65f, 0.65f, 0.65f);
	frameMaterial.shininess = 96.0f;   // tight highlight
	frameMaterial.tag = "frameMaterial";
	m_objectMaterials.push_back(frameMaterial);

	// White wood - low gloss
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(1.30f, 1.30f, 1.30f);
	woodMaterial.specularColor = glm::vec3(0.04f, 0.04f, 0.04f);
	woodMaterial.shininess = 6.0f;
	woodMaterial.tag = "woodMaterial";
	m_objectMaterials.push_back(woodMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting, 
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// Directional Light Sunlight
	m_pShaderManager->setVec3Value("directionalLight.direction", -0.2f, -1.0f, -0.3f);
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.55f, 0.55f, 0.58f);
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 1.00f, 0.97f, 0.90f);
	m_pShaderManager->setVec3Value("directionalLight.specular", 0.35f, 0.35f, 0.35f);
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);

	// Point Light 1 Sky fill
	m_pShaderManager->setVec3Value("pointLights[0].position", 6.0f, 12.0f, 10.0f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.20f, 0.22f, 0.28f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.35f, 0.45f, 0.70f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.10f, 0.10f, 0.10f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);

	// Point Light 2 ground bounce 
	m_pShaderManager->setVec3Value("pointLights[1].position", -4.0f, 1.0f, 2.0f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.05f, 0.04f, 0.03f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.12f, 0.10f, 0.08f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);
}
/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// define the materials for objects in the scene
	DefineObjectMaterials();

	// add and define the light sources for the scene
	SetupSceneLights();

	// load the textures for the 3D scene
	LoadSceneTextures();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	// Rim  torus
	m_basicMeshes->LoadTorusMesh();
	// tire torus
	m_basicMeshes->LoadExtraTorusMesh1(0.045f);
	// tapered cylinder for more realistic bike frame tubes
	m_basicMeshes->LoadTaperedCylinderMesh();
}
/***********************************************************
 *  RenderWheel()
 *
 *  Basis:
 *  Uses a polar sweep for spokes and layered primitives for
 *  hub, rim, and tire.  Radius drives all proportions.
 *
 *
 *  Parameters:
 *  wheelCenter, base rotations, and WHEEL_PARAMS for paramters.
 ***********************************************************/
void SceneManager::RenderWheel(
	glm::vec3 wheelCenter,
	float wheelBaseXRot,
	float wheelBaseYRot,
	float wheelBaseZRot,
	const WHEEL_PARAMS& wheelParams)
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/****************************************************************/
	/***				 Wheel parameters						  ***/
	/****************************************************************/

	// wheel sizing
	float wheelRadius = wheelParams.wheelRadius;
	const unsigned int spokeCount = wheelParams.spokeCount;

	// proportions derived from wheel radius
	float hubRadius = wheelRadius * wheelParams.hubRadiusFactor;
	float hubWidth = wheelRadius * wheelParams.hubWidthFactor;

	float spokeThickness = wheelRadius * wheelParams.spokeThicknessFactor;
	float rimTubeRadius = wheelRadius * wheelParams.rimTubeRadiusFactor;

	// simple clearances so spokes do not visually intersect the hub/rim
	float hubClearance = hubRadius + (wheelRadius * 0.02f);
	float spokeStartR = hubClearance;

	// spoke end radius extends beyond the rim to ensure the spoke end is visible in front of the rim texture
	float spokeEndR = wheelRadius * 1.25f;
	float spokeLength = spokeEndR - spokeStartR;

	float angleStepDeg = 360.0f / (float)spokeCount;
	float laceTiltDeg = wheelParams.laceTiltDeg;

	// torus template sizing assumption 
	float torusMajorLocal = 1.0f;

	// match the 0.045f used in LoadExtraTorusMesh1()
	float tireMinorLocal = 0.045f;
	float tireOuterLocal = torusMajorLocal + tireMinorLocal;

	// Scale tire so OUTER radius matches wheelRadius
	float tireScale = wheelRadius / tireOuterLocal;

	// Keep rim scale as it was before 
	float rimScale = wheelRadius / (torusMajorLocal + 0.25f);

	// Thin wheels to make closer to road bike thickness
	float wheelWidthSquash = wheelParams.wheelWidthSquash;

	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.                       ***/
	/******************************************************************/

	// HUB (cylinder)

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(hubRadius, hubWidth, hubRadius);

	// set the XYZ rotation for the mesh
	XrotationDegrees = wheelBaseXRot;
	YrotationDegrees = wheelBaseYRot;
	ZrotationDegrees = wheelBaseZRot;

	// set the XYZ position for the mesh
	// base-anchored cylinder: offset by -half hubWidth so the hub is centered on wheelCenter
	positionXYZ = glm::vec3(
		wheelCenter.x,
		wheelCenter.y,
		wheelCenter.z - (hubWidth * 0.5f)
	);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial(wheelParams.hubMaterialName);

	// set the texture for the next draw command
	SetShaderTexture(wheelParams.hubTextureName);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/******************************************************************/


	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.                       ***/
	/******************************************************************/

	// SPOKES

	for (unsigned int i = 0; i < spokeCount; i++)
	{
		float thetaDeg = (float)i * angleStepDeg;
		float thetaRad = glm::radians(thetaDeg);

		// spokes radiate in the wheel face plane (XY) 
		float dirX = cos(thetaRad);
		float dirY = sin(thetaRad);
		glm::vec3 spokeDir = glm::normalize(glm::vec3(dirX, dirY, 0.0f));

		// alternate slight tilt to suggest basic lacing 
		float tilt = (i % 2 == 0) ? laceTiltDeg : -laceTiltDeg;

		// place the spoke so its center sits halfway between start and end radii
		float spokeCenterR = spokeStartR + (spokeLength * 0.5f);

		// set the XYZ scale for the mesh
		// cylinder mesh extends along its local Y axis, so Y = spokeLength
		scaleXYZ = glm::vec3(spokeThickness * 2.0f, spokeLength, spokeThickness * 2.0f);

		// set the XYZ rotation for the mesh
		// rotate the spoke around the wheel's normal (Z) by thetaDeg
		XrotationDegrees = wheelBaseXRot + tilt + 90.0f;
		YrotationDegrees = wheelBaseYRot;
		ZrotationDegrees = wheelBaseZRot + thetaDeg - 90.0f;

		// set the XYZ position for the mesh
		positionXYZ = wheelCenter + (spokeDir * spokeCenterR);

		// set the transformations into memory to be used on the drawn meshes
		SetTransformations(
			scaleXYZ,
			XrotationDegrees,
			YrotationDegrees,
			ZrotationDegrees,
			positionXYZ);

		// set the active material into the shader
		SetShaderMaterial(wheelParams.spokeMaterialName);

		// set the texture for the next draw command
		SetShaderTexture(wheelParams.spokeTextureName);

		// draw the mesh with transformation values
		m_basicMeshes->DrawCylinderMesh();
	}
	/******************************************************************/


	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.                       ***/
	/******************************************************************/

	// RIM (torus)

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(rimScale, rimScale * wheelWidthSquash, rimScale); // thin wheel width

	// set the XYZ rotation for the mesh
	XrotationDegrees = wheelBaseXRot;
	YrotationDegrees = wheelBaseYRot;
	ZrotationDegrees = wheelBaseZRot;

	// set the XYZ position for the mesh
	positionXYZ = wheelCenter;

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial(wheelParams.rimMaterialName);

	// set the texture for the next draw command
	SetShaderTexture(wheelParams.rimTextureName);

	// draw the mesh with transformation values
	m_basicMeshes->DrawTorusMesh();
	/******************************************************************/


	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.                       ***/
	/******************************************************************/

	// TIRE (extra torus)

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(tireScale, tireScale * wheelWidthSquash, tireScale); // thin wheel width

	// set the XYZ rotation for the mesh
	XrotationDegrees = wheelBaseXRot;
	YrotationDegrees = wheelBaseYRot;
	ZrotationDegrees = wheelBaseZRot;

	// set the XYZ position for the mesh
	positionXYZ = wheelCenter;

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial(wheelParams.tireMaterialName);

	// set the texture for the next draw command
	SetShaderTexture(wheelParams.tireTextureName);

	// draw the mesh with transformation values
	m_basicMeshes->DrawExtraTorusMesh1();
	/******************************************************************/
}

/***********************************************************
 *  RenderFrameTube()
 *
 *  Basis:
 *  Aligns a base-anchored cylinder to segment using yaw (Z)
 *  and pitch (X), length sets Y scale.
 *
 *
 *  Parameters:
 *  startPos, endPos, frameParams, bUseTapered.
 ***********************************************************/
void SceneManager::RenderFrameTube(
	glm::vec3 startPos,
	glm::vec3 endPos,
	const FRAME_PARAMS& frameParams,
	bool bUseTapered)
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// compute vector from start to end
	glm::vec3 delta = endPos - startPos;
	float length = glm::length(delta);

	// normalize the delta to get the direction, and avoid values close to zero that can cause instabilities.
	if (length < 0.0001f)
	{
		return;
	}
	// normalize to get direction
	glm::vec3 dir = delta / length;

	/****************************************************************/
	/*** Rotation model											  ***/
	/****************************************************************/
	// - The cylinder mesh is base-anchored and extends along its local +Y axis.
	// - The cylinder's +Y axis should point from startPos to endPos.
	// Computes:
	// yaw around Z to align in X-Y plane
	// pitch around X to tilt toward +/- Z if needed

	float xyLen = sqrt((dir.x * dir.x) + (dir.y * dir.y));

	if (xyLen > 0.0001f)
	{
		// yaw: rotate around Z so the tube points in the right direction in X-Y plane
		ZrotationDegrees = glm::degrees(atan2(-dir.x, dir.y));
	}
	else
	{
		ZrotationDegrees = 0.0f;
	}

	// pitch: after yaw, tilt around X so the tube can rise/fall in Z as well
	XrotationDegrees = glm::degrees(atan2(dir.z, xyLen));

	/****************************************************************/
	/*** Scale model            								  ***/
	/****************************************************************/
	// Scale cross-section to emulate fake aero/oval tubes.
	// Y is the tube length because cylinder extends along +Y in local space.
	float tubeR = frameParams.tubeRadius;

	scaleXYZ = glm::vec3(
		tubeR * frameParams.aeroWidthFactor,
		length,
		tubeR * frameParams.aeroDepthFactor
	);

	// base-anchored cylinder: set base at startPos
	positionXYZ = startPos;

	// set transformations into shader
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set surface resources
	SetShaderMaterial(frameParams.frameMaterialName);
	SetShaderTexture(frameParams.frameTextureName);

	// draw the mesh
	if (bUseTapered == true)
	{
		m_basicMeshes->DrawTaperedCylinderMesh();
	}
	else
	{
		m_basicMeshes->DrawCylinderMesh();
	}
}

/***********************************************************
 *  RenderArcTubeXY()
 *
 *  Basis:
 *  Parametric circle in X-Y:
 *      x = cx + r cos(theta)
 *      y = cy + r sin(theta)
 *      z = cz + zOffset
 *
 *  Conceptual Inspiration:
 *  Inspired by torus mesh parameterization; torus uses
 *  two angles, but here sencond parameter is removed and
 *  cyclinders are subbed for segments a mesh wouoduse.
 *
 *  Parameters:
 *  arcCenter, arcRadius, startAngleDeg, endAngleDeg,
 *  zOffset, segmentCount, tubeParams.
 ***********************************************************/
void SceneManager::RenderArcTubeXY(
	glm::vec3 arcCenter,
	float arcRadius,
	float startAngleDeg,
	float endAngleDeg,
	float zOffset,
	int segmentCount,
	const FRAME_PARAMS& tubeParams)
{
	// Safety check: prevent division by zero and unnecessary work
	if (segmentCount < 1)
	{
		return;
	}

	// Compute angular step size between each segment.
	// This divides the total angular sweep evenly.
	float angleStep = (endAngleDeg - startAngleDeg) / (float)segmentCount;

	// Initialize current angle at start
	float thetaDeg = startAngleDeg;

	// Compute the first point on the arc using the parametric equations.
	glm::vec3 prevPoint;

	float thetaRad = glm::radians(thetaDeg);

	prevPoint.x = arcCenter.x + arcRadius * cos(thetaRad);
	prevPoint.y = arcCenter.y + arcRadius * sin(thetaRad);
	prevPoint.z = arcCenter.z + zOffset; // constant Z plane offset

	// for each segment, compute the next point along the arc and draw a tube between the previous and next points
	for (int i = 0; i < segmentCount; i++)
	{
		// Advance parameter ?
		thetaDeg += angleStep;
		thetaRad = glm::radians(thetaDeg);

		glm::vec3 nextPoint;

		// Parametric circle evaluation
		nextPoint.x = arcCenter.x + arcRadius * cos(thetaRad);
		nextPoint.y = arcCenter.y + arcRadius * sin(thetaRad);
		nextPoint.z = arcCenter.z + zOffset;

		// Draw a cylindrical segment between adjacent arc points.
		// This approximates a smooth curved tube.
		RenderFrameTube(prevPoint, nextPoint, tubeParams, false);

		// Move forward along the curve
		prevPoint = nextPoint;
	}
}

/***********************************************************
 *  ComputeFrameAnchors()
 *
 *  Basis:
 *  Uses wheel centers and wheelRadius to derivie key
 *  frame points (BB, head tube, seat tube, stays).
 *
 *
 *  Parameters:
 *  rear/front centers, wheelRadius, frameParams.
 ***********************************************************/
SceneManager::FRAME_ANCHORS SceneManager::ComputeFrameAnchors(
	glm::vec3 rearWheelCenter,
	glm::vec3 frontWheelCenter,
	float wheelRadius,
	const FRAME_PARAMS& frameParams)
{
	FRAME_ANCHORS anchors;
	anchors.rearWheelCenter = rearWheelCenter;
	anchors.frontWheelCenter = frontWheelCenter;
	anchors.wheelRadius = wheelRadius;

	anchors.wheelbase = frontWheelCenter.x - rearWheelCenter.x;

	// bottom bracket: forward of rear axle and below axle height
	anchors.bottomBracketPos = glm::vec3(
		rearWheelCenter.x + (anchors.wheelbase * 0.40f),
		wheelRadius * 0.55f,
		rearWheelCenter.z
	);

	// seat tube top: above and slightly behind BB 
	anchors.seatTubeTopPos = glm::vec3(
		anchors.bottomBracketPos.x - (anchors.wheelbase * 0.05f),
		wheelRadius * 2.75f,
		rearWheelCenter.z
	);

	// head tube: moved forward and higher
	anchors.headTubeTopPos = glm::vec3(
		frontWheelCenter.x - (anchors.wheelbase * 0.04f),
		wheelRadius * 2.70f,
		rearWheelCenter.z
	);

	// head tube bottom - based on head tube top, with length derived from wheel radius
	float headTubeLength = wheelRadius * 0.30f;
	anchors.headTubeBottomPos = glm::vec3(
		anchors.headTubeTopPos.x - (anchors.wheelbase * 0.02f),
		anchors.headTubeTopPos.y - headTubeLength,
		rearWheelCenter.z
	);

	// to seperate the stays and forks visually
	float tubeR = frameParams.tubeRadius;

	// narrower near bottom bracket / seat tube
	anchors.frameHalfWidth = tubeR * 0.05f;

	// slightly wider at the hubs than at the frame
	anchors.hubHalfWidth = tubeR * 1.20f;

	// left chain stay - from rear axle to bottom bracket
	anchors.rearAxleLeft = rearWheelCenter + glm::vec3(0.0f, 0.0f, +anchors.hubHalfWidth);
	anchors.rearAxleRight = rearWheelCenter + glm::vec3(0.0f, 0.0f, -anchors.hubHalfWidth);

	// right chain stay - from rear axle to bottom bracket
	anchors.bottomBracketLeft = anchors.bottomBracketPos + glm::vec3(0.0f, 0.0f, +anchors.frameHalfWidth);
	anchors.bottomBracketRight = anchors.bottomBracketPos + glm::vec3(0.0f, 0.0f, -anchors.frameHalfWidth);

	// left seat stay - from rear axle to seat tube top
	anchors.seatTubeTopLeft = anchors.seatTubeTopPos + glm::vec3(0.0f, 0.0f, +anchors.frameHalfWidth);
	// right seat stay - from rear axle to seat tube top
	anchors.seatTubeTopRight = anchors.seatTubeTopPos + glm::vec3(0.0f, 0.0f, -anchors.frameHalfWidth);

	// fork - from front axle to head tube bottom
	anchors.frontAxleLeft = frontWheelCenter + glm::vec3(0.0f, 0.0f, +anchors.hubHalfWidth);
	anchors.frontAxleRight = frontWheelCenter + glm::vec3(0.0f, 0.0f, -anchors.hubHalfWidth);

	// head tube bottom left and right for fork separation
	anchors.headTubeBottomLeft = anchors.headTubeBottomPos + glm::vec3(0.0f, 0.0f, +anchors.frameHalfWidth);
	anchors.headTubeBottomRight = anchors.headTubeBottomPos + glm::vec3(0.0f, 0.0f, -anchors.frameHalfWidth);

	return anchors;
}

/***********************************************************
 *  RenderBicycleFrame()
 *
 *  Basis:
 *  Draws frame tubes between anchor points using
 *  RenderFrameTube for each segment.
 *
 *
 *  Parameters:
 *  anchors and frameParams.
 ***********************************************************/
 void SceneManager::RenderBicycleFrame(
	 const FRAME_ANCHORS& anchors,
	 const FRAME_PARAMS& frameParams)
 {
	 /****************************************************************/
	 /***				 Bottom bracket shell					 ***/
	 /****************************************************************/
	 // declare the variables for the transformations
	 glm::vec3 scaleXYZ;
	 float XrotationDegrees = 0.0f;
	 float YrotationDegrees = 0.0f;
	 float ZrotationDegrees = 0.0f;
	 glm::vec3 positionXYZ;

	 float bbRadius = frameParams.tubeRadius * 1.08f;
	 float bbHalfWidth = frameParams.tubeRadius * 1.20f;
	 float bbWidth = bbHalfWidth * 2.0f;

	 // set the XYZ scale for the mesh
	 scaleXYZ = glm::vec3(bbRadius, bbWidth, bbRadius);

	 // set the XYZ rotation for the mesh
	 XrotationDegrees = 90.0f;
	 YrotationDegrees = 0.0f;
	 ZrotationDegrees = 0.0f;

	 // set the XYZ position for the mesh
	 positionXYZ = glm::vec3(
		 anchors.bottomBracketPos.x,
		 anchors.bottomBracketPos.y,
		 anchors.bottomBracketPos.z - (bbWidth * 0.5f)
	 );

	 // set the transformations into memory to be used on the drawn meshes
	 SetTransformations(
		 scaleXYZ,
		 XrotationDegrees,
		 YrotationDegrees,
		 ZrotationDegrees,
		 positionXYZ);

	 // set the active material into the shader
	 SetShaderMaterial(frameParams.frameMaterialName);

	 // set the texture for the next draw command
	 SetShaderTexture(frameParams.frameTextureName);

	 // draw the mesh with transformation values
	 m_basicMeshes->DrawCylinderMesh();
	 /****************************************************************/

	 /****************************************************************/
	 /***				 Draw frame tubes						   ***/
	 /****************************************************************/

	 // seat tube - from bottom bracket to seat tube top
	 RenderFrameTube(anchors.bottomBracketPos, anchors.seatTubeTopPos, frameParams, false);

	 // top tube - from seat tube top to head tube top
	 RenderFrameTube(anchors.seatTubeTopPos, anchors.headTubeTopPos, frameParams, false);

	 // down tube - from bottom bracket to head tube bottom, tapered for aero effect
	 RenderFrameTube(anchors.headTubeBottomPos, anchors.bottomBracketPos, frameParams, true);

	 // head tube - from head tube bottom to head tube top
	 RenderFrameTube(anchors.headTubeBottomPos, anchors.headTubeTopPos, frameParams, false);

	 // to sperate the stays and forks visually
	 // left chain stay - from rear axle to bottom bracket
	 RenderFrameTube(anchors.rearAxleLeft, anchors.bottomBracketLeft, frameParams, false);
	 RenderFrameTube(anchors.rearAxleRight, anchors.bottomBracketRight, frameParams, false);

	 // left seat stay - from rear axle to seat tube top
	 // render the seat stays
	 RenderFrameTube(anchors.rearAxleLeft, anchors.seatTubeTopLeft, frameParams, false);
	 RenderFrameTube(anchors.rearAxleRight, anchors.seatTubeTopRight, frameParams, false);

	// render the fork
	RenderFrameTube(anchors.frontAxleLeft, anchors.headTubeBottomLeft, frameParams, false);
	RenderFrameTube(anchors.frontAxleRight, anchors.headTubeBottomRight, frameParams, false);
 }

/***********************************************************
 *  RenderSeatpostAndSaddle()
 *
 *  Basis:
 *  Seatpost is a vertical cylinder; saddle is a triangle
 *  of tubes on top 
 *
 *
 *  Parameters:
 *  anchors and frameParams.
 ***********************************************************/
void SceneManager::RenderSeatpostAndSaddle(
	const FRAME_ANCHORS& anchors,
	const FRAME_PARAMS& frameParams)
{
	/****************************************************************/
	/***				 Seatpost								  ***/
	/****************************************************************/
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	float seatpostRadius = frameParams.tubeRadius * 0.55f;
	float seatpostHeight = anchors.wheelRadius * 0.60f;

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(seatpostRadius, seatpostHeight, seatpostRadius);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = anchors.seatTubeTopPos;

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial("rimMaterial");

	// set the texture for the next draw command
	SetShaderTexture("rim");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/****************************************************************/

	/****************************************************************/
	/***				 Saddle									  ***/
	/****************************************************************/
	glm::vec3 seatpostTop = anchors.seatTubeTopPos + glm::vec3(0.0f, seatpostHeight, 0.0f);
	
	// saddle tubes - use same material as tire to suggest rubbery texture, and smaller radius than frame tubes
	FRAME_PARAMS saddleParams = frameParams;
	saddleParams.tubeRadius = frameParams.tubeRadius * 0.30f;
	saddleParams.frameMaterialName = "tireMaterial";
	saddleParams.frameTextureName = "tire";
	
	// saddle geometry - simple triangle with nose forward and rear wider than front, sized based on wheel radius
	glm::vec3 saddleNose = seatpostTop + glm::vec3(anchors.wheelRadius * 0.15f, anchors.wheelRadius * 0.02f, 0.0f);
	glm::vec3 saddleRearLeft = seatpostTop + glm::vec3(-anchors.wheelRadius * 0.10f, anchors.wheelRadius * 0.05f, anchors.wheelRadius * 0.08f);
	glm::vec3 saddleRearRight = seatpostTop + glm::vec3(-anchors.wheelRadius * 0.10f, anchors.wheelRadius * 0.05f, -anchors.wheelRadius * 0.08f);

	RenderFrameTube(saddleNose, saddleRearLeft, saddleParams, false);
	RenderFrameTube(saddleRearLeft, saddleRearRight, saddleParams, false);
	RenderFrameTube(saddleRearRight, saddleNose, saddleParams, false);
	/****************************************************************/
}

/***********************************************************
 *  RenderSteeringAssembly()
 *
 *  Basis:
 *  Steerer (I know this is a bad name i picked it and without thinking 
 *  and don't want to deal with changing it) and stem are short tubes; bars are an XY arc
 *  with z offsets for left/right.
 *
 *
 *  Parameters:
 *  anchors and frameParams.
 ***********************************************************/
void SceneManager::RenderSteeringAssembly(
	const FRAME_ANCHORS& anchors,
	const FRAME_PARAMS& frameParams)
{
	/****************************************************************/
	/***                 Stem + Handlebars			              ***/
	/****************************************************************/
	FRAME_PARAMS metalParams = frameParams; // start with frame params but modify for metal stem and handlebars
	metalParams.tubeRadius = frameParams.tubeRadius * 0.45f;// thinner tubes for stem and handlebars
	// use the same texture and material as the hub for a metallic look
	metalParams.frameMaterialName = "hubMaterial";
	metalParams.frameTextureName = "hub";

	/****************************************************************/
	/***           Steerer / Headset Tube - Fork Top              ***/
	/****************************************************************/
	glm::vec3 steererBase = anchors.headTubeTopPos; 
	float steererHeight = anchors.wheelRadius * 0.14f * 1.50f;
	glm::vec3 steererTop = steererBase + glm::vec3(0.0f, steererHeight, 0.0f);
	// render the steerer tube as a continuation of the head tube, with the same material and texture as the frame
	RenderFrameTube(steererBase, steererTop, metalParams, false);

	/****************************************************************/
	/***                         Stem                              ***/
	/****************************************************************/
	float stemLen = anchors.wheelRadius * 0.18f * 1.50f;
	float stemRise = anchors.wheelRadius * 0.04f * 1.50f;
	glm::vec3 stemTip = steererTop + glm::vec3(stemLen, stemRise, 0.0f);

	RenderFrameTube(steererTop, stemTip, metalParams, false);

	/****************************************************************/
	/***    Drop style Handlebars - Metal Center & Rubber wrap    ***/
	/****************************************************************/
	glm::vec3 handlebarCenter = stemTip;

	float halfWidth = anchors.wheelRadius * 0.45f;
	// handlebar ends - use metal material to suggest exposed metal where rubber wrap would not cover
	FRAME_PARAMS rubberParams = metalParams;
	rubberParams.frameMaterialName = "tireMaterial";
	rubberParams.frameTextureName = "tire";

	// clamps that attach the handlebar to the stem 
	float centerClampHalf = anchors.wheelRadius * 0.08f;
	glm::vec3 clampLeft = handlebarCenter + glm::vec3(0.0f, 0.0f, +centerClampHalf);
	glm::vec3 clampRight = handlebarCenter + glm::vec3(0.0f, 0.0f, -centerClampHalf);

	// render the clamps as part of the metal handlebar assembly
	RenderFrameTube(clampLeft, clampRight, metalParams, false);

	// handlebar drops - use an arc to create the curved shape of the drops, with rubber material
	float arcRadius = anchors.wheelRadius * 0.30f;
	float dropZ = halfWidth * 0.55f;
	glm::vec3 topsEndLeft = glm::vec3(handlebarCenter.x, handlebarCenter.y, handlebarCenter.z + dropZ);
	glm::vec3 topsEndRight = glm::vec3(handlebarCenter.x, handlebarCenter.y, handlebarCenter.z - dropZ);

	// render the straight sections of the handlebar from the stem to the start of the drops, with rubber material
	RenderFrameTube(clampLeft, topsEndLeft, rubberParams, false);
	RenderFrameTube(clampRight, topsEndRight, rubberParams, false);

	// render the drops as arcs in the X-Y plane, with rubber material
	glm::vec3 arcCenter = glm::vec3(topsEndLeft.x, topsEndLeft.y - arcRadius, handlebarCenter.z);
	float startAngleDeg = 90.0f;
	float endAngleDeg = -90.0f;
	int segmentCount = 100;

	// render right drops +z - use the same arc center and angles for both sides
	RenderArcTubeXY(
		arcCenter,
		arcRadius,
		startAngleDeg,
		endAngleDeg,
		+dropZ,
		segmentCount,
		rubberParams);

		// render left drops -z - use the same arc center and angles for both sides
	RenderArcTubeXY(
		arcCenter,
		arcRadius,
		startAngleDeg,
		endAngleDeg,
		-dropZ,
		segmentCount,
		rubberParams);
}

/***********************************************************
 *  RenderPedals()
 *
 *  Basis:
 *  Crank arms are short tubes in Y with small Z offsets;
 *  pedals are flat boxes at ends.
 *
 *
 *  Parameters:
 *  anchors and frameParams.
 ***********************************************************/
void SceneManager::RenderPedals(
	const FRAME_ANCHORS& anchors,
	const FRAME_PARAMS& frameParams)
{
	/****************************************************************/
	/***				 Crank arms								  ***/
	/****************************************************************/
	float crankLength = anchors.wheelRadius * 0.40f;
	float crankThickness = frameParams.tubeRadius * 0.35f;
	float pedalWidth = anchors.wheelRadius * 0.22f;
	float pedalThicknessBase = frameParams.tubeRadius * 0.35f;
	float pedalThickness = pedalThicknessBase * 9.0f;
	float pedalHeight = anchors.wheelRadius * 0.02f;

	FRAME_PARAMS metalParams = frameParams;
	metalParams.tubeRadius = crankThickness;
	metalParams.frameMaterialName = "hubMaterial";
	metalParams.frameTextureName = "hub";

	float crankZOffset = frameParams.tubeRadius * 1.5f;
	glm::vec3 leftCrankStart = anchors.bottomBracketPos + glm::vec3(0.0f, 0.0f, +crankZOffset);
	glm::vec3 rightCrankStart = anchors.bottomBracketPos + glm::vec3(0.0f, 0.0f, -crankZOffset);
	glm::vec3 leftCrankEnd = leftCrankStart + glm::vec3(0.0f, -crankLength, 0.0f);
	glm::vec3 rightCrankEnd = rightCrankStart + glm::vec3(0.0f, +crankLength, 0.0f);

	RenderFrameTube(leftCrankStart, leftCrankEnd, metalParams, false);
	RenderFrameTube(rightCrankStart, rightCrankEnd, metalParams, false);

	/****************************************************************/
	/***				 Pedals									  ***/
	/****************************************************************/
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(pedalWidth, pedalHeight, pedalThickness);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = leftCrankEnd + glm::vec3(0.0f, 0.0f, (pedalThickness - pedalThicknessBase) * 0.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial("rimMaterial");

	// set the texture for the next draw command
	SetShaderTexture("rim");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(pedalWidth, pedalHeight, pedalThickness);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = rightCrankEnd + glm::vec3(0.0f, 0.0f, -(pedalThickness - pedalThicknessBase) * 0.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial("rimMaterial");

	// set the texture for the next draw command
	SetShaderTexture("rim");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();
	/****************************************************************/
}
/****************************************************************/
/*** RenderPerson()												 ***/
/*** TThis method draws a simple person behind the barrier.    ***/
/******************************************************************/

void SceneManager::RenderPerson(

	glm::vec3 position,

	float scale,

	glm::vec3 shirtColor,

	glm::vec3 pantsColor)
{

	glm::vec3 scaleXYZ;

	float XrotationDegrees = 0.0f;

	float YrotationDegrees = 0.0f;

	float ZrotationDegrees = 0.0f;

	glm::vec3 positionXYZ;


	SetShaderMaterial("barrierPlastic"); 


	// Left Leg (Cylinder)

	scaleXYZ = glm::vec3(0.2f * scale, 1.2f * scale, 0.2f * scale);

	XrotationDegrees = 0.0f; YrotationDegrees = 0.0f; ZrotationDegrees = 0.0f;

	positionXYZ = position + glm::vec3(-0.2f * scale, 0.0f, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	SetShaderColor(pantsColor.r, pantsColor.g, pantsColor.b, 1.0f);

	m_basicMeshes->DrawCylinderMesh();


	// Right Leg

	positionXYZ = position + glm::vec3(0.2f * scale, 0.0f, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	m_basicMeshes->DrawCylinderMesh();


	// Torso (Cylinder)

	scaleXYZ = glm::vec3(0.4f * scale, 1.3f * scale, 0.25f * scale);

	positionXYZ = position + glm::vec3(0.0f, 1.1f * scale, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	SetShaderColor(shirtColor.r, shirtColor.g, shirtColor.b, 1.0f);

	m_basicMeshes->DrawCylinderMesh();


	// Head (Sphere) doesn't use base anchor, 

	scaleXYZ = glm::vec3(0.3f * scale, 0.3f * scale, 0.3f * scale);

	positionXYZ = position + glm::vec3(0.0f, 2.7f * scale, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	SetShaderColor(0.85f, 0.65f, 0.5f, 1.0f); // skin tone

	m_basicMeshes->DrawSphereMesh();


	// Left Arm (Cheering)

	scaleXYZ = glm::vec3(0.15f * scale, 0.9f * scale, 0.15f * scale);

	XrotationDegrees = 0.0f;

	YrotationDegrees = 0.0f;

	ZrotationDegrees = 30.0f; // Tilt outwards

	// anchored at shoulder height

	positionXYZ = position + glm::vec3(-0.4f * scale, 2.2f * scale, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	SetShaderColor(shirtColor.r, shirtColor.g, shirtColor.b, 1.0f);

	m_basicMeshes->DrawCylinderMesh();


	// Right Arm (Cheering)

	ZrotationDegrees = -40.0f; // Tilt outwards, slightly different angle

	positionXYZ = position + glm::vec3(0.4f * scale, 2.1f * scale, 0.0f);

	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	m_basicMeshes->DrawCylinderMesh();
}

/***********************************************************
 *  RenderBicycle()
 *
 *  Basis:
 *  Assembles wheels, frame, seat, steering, pedals
 *  from shared wheel size and anchors.
 *
 *
 *  Parameters:
 *  bikePosition used as origin for whole bicycle.
 ***********************************************************/

void SceneManager::RenderBicycle(glm::vec3 bikePosition)
{
	/****************************************************************/
	/***				 Bicycle placement						  ***/
	/****************************************************************/

	// wheel stands upright
	float wheelBaseXRot = 90.0f;
	float wheelBaseYRot = 0.0f;
	float wheelBaseZRot = 0.0f;

	WHEEL_PARAMS wheelParams;
	// fixed wheel radius - decoupled from plane size (Failed Experiment).
	wheelParams.wheelRadius = 0.75f;
	wheelParams.spokeCount = 28;

	// wheel centers
	glm::vec3 rearWheelCenter = glm::vec3(
		bikePosition.x,
		wheelParams.wheelRadius,
		bikePosition.z
	);

	// basic road bike wheelbase 
	float bikeWheelbase = wheelParams.wheelRadius * 3.4f;

	glm::vec3 frontWheelCenter = glm::vec3(
		rearWheelCenter.x + bikeWheelbase,
		rearWheelCenter.y,
		rearWheelCenter.z
	);

	/****************************************************************/
	/***				 Draw wheels							  ***/
	/****************************************************************/

	RenderWheel(
		rearWheelCenter,
		wheelBaseXRot,
		wheelBaseYRot,
		wheelBaseZRot,
		wheelParams);

	RenderWheel(
		frontWheelCenter,
		wheelBaseXRot,
		wheelBaseYRot,
		wheelBaseZRot,
		wheelParams);

	/****************************************************************/
	/***				 Draw frame								  ***/
	/****************************************************************/

	FRAME_PARAMS frameParams;

	// scale tube radius off wheel size so it remains proportional
	frameParams.tubeRadius = wheelParams.wheelRadius * 0.06f;

	// aero shaping 
	frameParams.aeroWidthFactor = 1.7f;
	frameParams.aeroDepthFactor = 0.65f;

	// use the same material and texture for all frame tubes for simplicity
	frameParams.frameMaterialName = "frameMaterial";
	frameParams.frameTextureName = "frame";

	FRAME_ANCHORS anchors = ComputeFrameAnchors(
		rearWheelCenter,
		frontWheelCenter,
		wheelParams.wheelRadius,
		frameParams);

	RenderBicycleFrame(
		anchors,
		frameParams);

	RenderSeatpostAndSaddle(
		anchors,
		frameParams);

	RenderSteeringAssembly(
		anchors,
		frameParams);

	RenderPedals(
		anchors,
		frameParams);
}
/****************************************************************/
/*** RenderScene()											  ***/
/*** This method is used for rendering the 3D scene by        ***/
/***  transforming and drawing the basic 3D shapes			   ***/
/******************************************************************/

void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(30.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-5.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(0.412, 0.412, 0.412, 1);

	// set the active material into the shader
	SetShaderMaterial("roadMaterial");

	// set the texture for the next draw command
	SetShaderTexture("road");
	SetTextureUVScale(40.0f, 20.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(30.0f, 1.0f, 15.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-5.0f, 1.0f, -15.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(0.412, 0.412, 0.412, 1);

	// set the active material into the shader
	SetShaderMaterial("brickMaterial");

	// set the texture for the next draw command
	SetShaderTexture("brick");
	SetTextureUVScale(8.0f, 4.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (left sidesemi-transparent dark arch)
	scaleXYZ = glm::vec3(2.45f, 1.0f, 10.f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.38f, 1.5f, -6.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	// set the active material into the shader
	SetShaderMaterial("darkPlasticMaterial");

	// set the color values into the shader - alpha channel is for transparency
	SetShaderColor(0.05f, 0.05f, 0.05f, 0.9f);


	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (left arch banner
	scaleXYZ = glm::vec3(2.0f, 1.0f, 2.50f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.38f, 7.0f, -5.9f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color values into the shader
	//SetShaderColor(1, 0.647, 0, 1);

	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the texture for the next draw command
	SetShaderTexture("banner");
	SetTextureUVScale(1.0f, 4.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (White divider in the arch)
	scaleXYZ = glm::vec3(0.15f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(5.0f, 1.5f, -6.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the color values into the shader
	SetShaderColor(1, 0.980, 0.980, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (right  side semi-transparent dark arch)
	scaleXYZ = glm::vec3(2.45f, 1.0f, 10.f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(7.62f, 5.0f, -6.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	// set the active material into the shader
	SetShaderMaterial("darkPlasticMaterial");

	// set the color values into the shader - alpha channel is for transparency
	SetShaderColor(0.05f, 0.05f, 0.05f, 0.9f);


	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (Rightmost continental banner on the arch)
	scaleXYZ = glm::vec3(2.0f, 1.0f, 2.50f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(7.62f, 7.0f, -5.9f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color values into the shader
	//SetShaderColor(1, 0.647, 0, 1);

	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the texture for the next draw command
	SetShaderTexture("banner");
	SetTextureUVScale(1.0f, 4.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (Rightmost orange barrier)
	scaleXYZ = glm::vec3(4.75f, 1.0f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(15.0f, 1.5f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color values into the shader
	//SetShaderColor(1, 0.647, 0, 1);

	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the texture for the next draw command
	SetShaderTexture("barrier");
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	/****************************************************************/

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (Left Black side of Black and white Barrier)
	scaleXYZ = glm::vec3(2.3f, 1.0f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.38f, 1.5f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color values into the shader
	//SetShaderColor(1, 0.647, 0, 1);

	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the texture for the next draw command
	SetShaderTexture("barrier2");
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (White divider of Black and white Barrier)
	scaleXYZ = glm::vec3(0.3f, 1.0f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(5.0f, 1.5f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the color values into the shader
	SetShaderColor(1, 0.980, 0.980, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (Right Black side of Black and white Barrier)
	scaleXYZ = glm::vec3(2.3f, 1.0f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(7.62f, 1.5f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color values into the shader
	//SetShaderColor(1, 0.647, 0, 1);

	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the texture for the next draw command
	SetShaderTexture("barrier2");
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	/****************************************************************/

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (Second to the left orange Barrier)
	scaleXYZ = glm::vec3(4.80f, 1.0f, 1.50f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-5.0f, 1.5f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color values into the shader
	//SetShaderColor(1, 0.647, 0, 1);

	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the texture for the next draw command
	SetShaderTexture("barrier");
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	/****************************************************************/

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (Leftmost orange Barrier)
	scaleXYZ = glm::vec3(4.80f, 1.0f, 1.50f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-15.0f, 1.5f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	
	// set the color values into the shader
	//SetShaderColor(1, 0.647, 0, 1);
		
	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the texture for the next draw command
	SetShaderTexture("barrier");
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	/******************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (Leftmost orange Barrier)
	scaleXYZ = glm::vec3(4.80f, 1.0f, 1.50f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-25.0f, 1.5f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color values into the shader
	//SetShaderColor(1, 0.647, 0, 1);

	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the texture for the next draw command
	SetShaderTexture("barrier");
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (White part of finish line)
	scaleXYZ = glm::vec3(1.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(5.0f, 0.1f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial("vinylMaterial");

	// set the color values into the shader
	SetShaderColor(1, 0.980, 0.980, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/


	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (Black part of finish line)
	scaleXYZ = glm::vec3(0.15f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(5.0f, 0.11f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial("vinylMaterial");

	// set the color values into the shader
	SetShaderColor(0, 0, 0, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
	
	/****************************************************************/
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh (White divider of Black and white Barrier)
	scaleXYZ = glm::vec3(0.3f, 1.0f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(5.0f, 1.5f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the active material into the shader
	SetShaderMaterial("barrierPlastic");

	// set the color values into the shader
	SetShaderColor(1, 0.980, 0.980, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	/****************************************************************/
	/***				 Draw Cheering People					  ***/
	/****************************************************************/
	glm::vec3 shirtColors[] = {
		glm::vec3(0.8f, 0.2f, 0.2f),
		glm::vec3(0.2f, 0.8f, 0.2f),
		glm::vec3(0.2f, 0.2f, 0.8f),
		glm::vec3(0.9f, 0.8f, 0.1f),
		glm::vec3(1.0f, 0.5f, 0.1f)
	};
	int shirtCount = 5;
	glm::vec3 darkPants(0.1f, 0.1f, 0.2f);
	glm::vec3 jeans(0.1f, 0.3f, 0.6f);

	for (int row = 0; row < 5; row++)
	{
		float z = -6.0f + row * (-1.2f);
		float y = (row < 2) ? 0.0f : (float)(row - 1) * 1.5f;
		for (int col = -12; col <= 5; col++)
		{
			float x = (float)col * 2.0f;
			if (x > -2.0f)
				continue;
			int shirtIndex = (row * 7 + col + 1000) % shirtCount;
			glm::vec3 shirt = shirtColors[shirtIndex];
			glm::vec3 pants = ((row + col) % 2 == 0) ? jeans : darkPants;
			float scale = 1.6f + 0.1f * (float)((row + col + 1000) % 4);

			RenderPerson(glm::vec3(x, y, z), scale, shirt, pants);
		}
	}
	/****************************************************************/

	/****************************************************************/
	/***				 Bicycle placement						 ***/
	/****************************************************************/
	RenderBicycle(glm::vec3(0.70f, 0.0f, -2.7f)); // farthest back
	RenderBicycle(glm::vec3(0.80f, 0.0f, -1.0f)); // middle - winner
	RenderBicycle(glm::vec3(-2.5f, 0.0f, 3.3f)); // closest to camera - third place - further back


}
