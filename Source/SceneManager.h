///////////////////////////////////////////////////////////////////////////////
// scenemanager.h
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"

#include <string>
#include <vector>

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:
	// constructor
	SceneManager(ShaderManager *pShaderManager);
	// destructor
	~SceneManager();

	struct TEXTURE_INFO
	{
		std::string tag;
		uint32_t ID;
	};

	struct OBJECT_MATERIAL
	{
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		std::string tag;
	};

private:
	// pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// pointer to basic shapes object
	ShapeMeshes* m_basicMeshes;
	// total number of loaded textures
	int m_loadedTextures;
	// loaded textures info
	TEXTURE_INFO m_textureIDs[16];
	// defined object materials
	std::vector<OBJECT_MATERIAL> m_objectMaterials;

	// load texture images and convert to OpenGL texture data
	bool CreateGLTexture(const char* filename, std::string tag);
	// bind loaded OpenGL textures to slots in memory
	void BindGLTextures();
	// free the loaded OpenGL textures
	void DestroyGLTextures();
	// find a loaded texture by tag
	int FindTextureID(std::string tag);
	int FindTextureSlot(std::string tag);
	// find a defined material by tag
	bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);

	// set the transformation values 
	// into the transform buffer
	void SetTransformations(
		glm::vec3 scaleXYZ,
		float XrotationDegrees,
		float YrotationDegrees,
		float ZrotationDegrees,
		glm::vec3 positionXYZ);

	// set the color values into the shader
	void SetShaderColor(
		float redColorValue,
		float greenColorValue,
		float blueColorValue,
		float alphaValue);

	// set the texture data into the shader
	void SetShaderTexture(
		std::string textureTag);

	// set the UV scale for the texture mapping
	void SetTextureUVScale(
		float u, float v);

	// set the object material into the shader
	void SetShaderMaterial(
		std::string materialTag);
	
	/***********************************************************
	 *  WHEEL PARAMETERS
	 *
	 *  This struct defines all configurable properties required
	 *  to render a reusable wheel object.  Dimensions are derived
	 *  proportionally from wheelRadius so the wheel scales cleanly
	 *  when resized.
	 ***********************************************************/
	struct WHEEL_PARAMS
	{
		// overall wheel sizing
		float wheelRadius = 0.8f;
		unsigned int spokeCount = 28;

		//  geometry factors - relative to wheelRadius)
		float hubRadiusFactor = 0.04f;
		float hubWidthFactor = 0.08f;
		float spokeThicknessFactor = 0.003f;
		float rimTubeRadiusFactor = 0.06f;

		// visual tuning controls
		float laceTiltDeg = 1.0f;
		float wheelWidthSquash = 0.18f;

		/***********************************************************
		 *  Surface Resource Names
		 *
		 *  These strings correspond to material and texture names
		 *  Declared in:
		 *     DefineObjectMaterials()
		 *     LoadSceneTextures()
		 ***********************************************************/

		 // hub surface
		std::string hubMaterialName = "hubMaterial";
		std::string hubTextureName = "hub";

		// spoke surface
		std::string spokeMaterialName = "spokeMaterial";
		std::string spokeTextureName = "spoke";

		// rim surface
		std::string rimMaterialName = "rimMaterial";
		std::string rimTextureName = "rim";

		// tire surface
		std::string tireMaterialName = "tireMaterial";
		std::string tireTextureName = "tire";
	};

	/***********************************************************
	 *  FRAME PARAMETERS
	 *
	 *  This struct defines configurable properties required
	 *  to render a reusable bicycle frame.
	 ***********************************************************/
	struct FRAME_PARAMS
	{
		// tube sizing - scaled from wheelRadius in RenderScene
		float tubeRadius = 0.04f;

		// aero shaping factors - for realism
		float aeroWidthFactor = 1.6f;   // wider dimension
		float aeroDepthFactor = 0.7f;   // thinner dimension

		// surface resources 
		std::string frameMaterialName = "frameMaterial";
		std::string frameTextureName = "frame";
	};

	/***********************************************************
	 *  FRAME ANCHORS
	 *
	 *  This struct defines anchor points required to render
	 *  a reusable bicycle frame.
	 ***********************************************************/
	struct FRAME_ANCHORS
	{
		glm::vec3 rearWheelCenter;
		glm::vec3 frontWheelCenter;
		float wheelRadius;
		float wheelbase;
		glm::vec3 bottomBracketPos;
		glm::vec3 seatTubeTopPos;
		glm::vec3 headTubeTopPos;
		glm::vec3 headTubeBottomPos;
		glm::vec3 rearAxleLeft, rearAxleRight;
		glm::vec3 frontAxleLeft, frontAxleRight;
		glm::vec3 bottomBracketLeft, bottomBracketRight;
		glm::vec3 seatTubeTopLeft, seatTubeTopRight;
		glm::vec3 headTubeBottomLeft, headTubeBottomRight;
		float frameHalfWidth;
		float hubHalfWidth;
	};

	/***********************************************************
	 *  ComputeFrameAnchors()
	 *
	 *  This method computes and returns frame anchor points.
	 ***********************************************************/
	FRAME_ANCHORS ComputeFrameAnchors(
		glm::vec3 rearWheelCenter,
		glm::vec3 frontWheelCenter,
		float wheelRadius,
		const FRAME_PARAMS& frameParams);

	/***********************************************************
	 *  RenderFrameTube()
	 *
	 *  This method draws a tube segment between two points.
	 ***********************************************************/
	void RenderFrameTube(
		glm::vec3 startPos,
		glm::vec3 endPos,
		const FRAME_PARAMS& frameParams,
		bool bUseTapered);

	/***********************************************************
	 *  RenderArcTubeXY()
	 *
	 *  This method draws a segmented arc in the X-Y plane.
	 ***********************************************************/
	void RenderArcTubeXY(
		glm::vec3 arcCenter,
		float arcRadius,
		float startAngleDeg,
		float endAngleDeg,
		float zOffset,
		int segmentCount,
		const FRAME_PARAMS& tubeParams);

	/***********************************************************
	 *  ComputeArcPointXY()
	 *
	 *  This method computes a point on an arc in the X-Y plane.
	 ***********************************************************/
	glm::vec3 ComputeArcPointXY(
		glm::vec3 arcCenter,
		float arcRadius,
		float angleDeg,
		float zOffset);

	/***********************************************************
	 *  RenderBicycleFrame()
	 *
	 *  This method draws a basic bicycle frame using the
	 *  frame anchor references.
	 ***********************************************************/
	void RenderBicycleFrame(
		const FRAME_ANCHORS& anchors,
		const FRAME_PARAMS& frameParams);

	/***********************************************************
	 *  RenderSeatpostAndSaddle()
	 *
	 *  This method draws the bicycle seatpost and saddle.
	 ***********************************************************/
	void RenderSeatpostAndSaddle(
		const FRAME_ANCHORS& anchors,
		const FRAME_PARAMS& frameParams);

	/***********************************************************
	 *  RenderSteeringAssembly()
	 *
	 *  This method draws the bicycle stem and handlebars.
	 ***********************************************************/
	void RenderSteeringAssembly(
		const FRAME_ANCHORS& anchors,
		const FRAME_PARAMS& frameParams);

	/***********************************************************
	 *  RenderPedals()
	 *
	 *  This method draws the bicycle crank arms and pedals.
	 ***********************************************************/
	void RenderPedals(
		const FRAME_ANCHORS& anchors,
		const FRAME_PARAMS& frameParams);

	/***********************************************************
	 *  RenderPerson()
	 *
	 *  This method draws a simple person behind the barrier.
	 ***********************************************************/
	void RenderPerson(
		glm::vec3 position,
		float scale,
		glm::vec3 shirtColor,
		glm::vec3 pantsColor);

	/***********************************************************
	 *  RenderBicycle()
	 *
	 *  This method assembles the bicycle components into a
	 *  single reusable draw call.
	 ***********************************************************/
	void RenderBicycle(glm::vec3 bikePosition);

	/***********************************************************
	 *  RenderWheel()
	 *
	 *  This method draws a wheel at a given center position using
	 *  the provided rotation values and wheel parameters.
	 ***********************************************************/
	void RenderWheel(
		glm::vec3 wheelCenter,
		float wheelBaseXRot,
		float wheelBaseYRot,
		float wheelBaseZRot,
		const WHEEL_PARAMS& wheelParams);

public:

	void LoadSceneTextures();

	// The following methods are for defining the materials and lighting
	void DefineObjectMaterials();
	void SetupSceneLights();

	// The following methods are for the students to 
	// customize for their own 3D scene
	void PrepareScene();
	void RenderScene();

};
