# Bicycle Equations Reference

This file lists the key equations used in bicycle rendering and where they appear in code.

## Wheel Spokes (RenderWheel)
- Angle step:
  - `float angleStepDeg = 360.0f / (float)spokeCount;`
- Angle per spoke:
  - `float thetaDeg = (float)i * angleStepDeg;`
  - `float thetaRad = glm::radians(thetaDeg);`
- Spoke direction in wheel face plane:
  - `float dirX = cos(thetaRad);`
  - `float dirY = sin(thetaRad);`
  - `glm::vec3 spokeDir = glm::normalize(glm::vec3(dirX, dirY, 0.0f));`
- Spoke placement radius:
  - `float spokeCenterR = spokeStartR + (spokeLength * 0.5f);`
- Spoke orientation (rotation):
  - `XrotationDegrees = wheelBaseXRot + tilt + 90.0f;`
  - `ZrotationDegrees = wheelBaseZRot + thetaDeg - 90.0f;`

## Frame Tube Alignment (RenderFrameTube)
- Delta and length:
  - `glm::vec3 delta = endPos - startPos;`
  - `float length = glm::length(delta);`
- Direction normalization:
  - `glm::vec3 dir = delta / length;`
- XY projection length:
  - `float xyLen = sqrt((dir.x * dir.x) + (dir.y * dir.y));`
- Yaw (around Z):
  - `ZrotationDegrees = glm::degrees(atan2(-dir.x, dir.y));`
- Pitch (around X):
  - `XrotationDegrees = glm::degrees(atan2(dir.z, xyLen));`
- Scale (aero tube cross section):
  - `scaleXYZ = glm::vec3(tubeR * frameParams.aeroWidthFactor, length, tubeR * frameParams.aeroDepthFactor);`

## Handlebar Arc (RenderArcTubeXY)
- Parametric circle in X-Y plane:
  - `thetaRad = glm::radians(thetaDeg);`
  - `x = arcCenter.x + arcRadius * cos(thetaRad);`
  - `y = arcCenter.y + arcRadius * sin(thetaRad);`
  - `z = arcCenter.z + zOffset;`
- Segment stepping:
  - `angleStep = (endAngleDeg - startAngleDeg) / (float)segmentCount;`

## Wheelbase and Frame Anchors (ComputeFrameAnchors)
- Wheelbase:
  - `anchors.wheelbase = frontWheelCenter.x - rearWheelCenter.x;`
- Bottom bracket:
  - `anchors.bottomBracketPos = glm::vec3(rearWheelCenter.x + (anchors.wheelbase * 0.40f), wheelRadius * 0.55f, rearWheelCenter.z);`
- Seat tube top:
  - `anchors.seatTubeTopPos = glm::vec3(anchors.bottomBracketPos.x - (anchors.wheelbase * 0.05f), wheelRadius * 2.75f, rearWheelCenter.z);`
- Head tube top:
  - `anchors.headTubeTopPos = glm::vec3(frontWheelCenter.x - (anchors.wheelbase * 0.04f), wheelRadius * 2.70f, rearWheelCenter.z);`
- Head tube bottom:
  - `headTubeLength = wheelRadius * 0.30f;`
  - `anchors.headTubeBottomPos = glm::vec3(anchors.headTubeTopPos.x - (anchors.wheelbase * 0.02f), anchors.headTubeTopPos.y - headTubeLength, rearWheelCenter.z);`
- Hub/frame offsets:
  - `anchors.frameHalfWidth = frameParams.tubeRadius * 0.0f;`
  - `anchors.hubHalfWidth = frameParams.tubeRadius * 1.20f;`

## Seatpost and Saddle (RenderSeatpostAndSaddle)
- Seatpost height:
  - `float seatpostHeight = anchors.wheelRadius * 0.60f;`
- Saddle triangle points:
  - `saddleNose = seatpostTop + vec3(0.15R, 0.02R, 0.0);`
  - `saddleRearLeft = seatpostTop + vec3(-0.10R, 0.05R, 0.08R);`
  - `saddleRearRight = seatpostTop + vec3(-0.10R, 0.05R, -0.08R);`

## Steering Assembly Dimensions (RenderSteeringAssembly)
- Steerer height:
  - `float steererHeight = anchors.wheelRadius * 0.14f * 1.50f;`
- Stem length/rise:
  - `float stemLen = anchors.wheelRadius * 0.18f * 1.50f;`
  - `float stemRise = anchors.wheelRadius * 0.04f * 1.50f;`
- Handlebar tops and drop arc:
  - `dropZ = halfWidth * 0.55f;`
  - `topsEndLeft = vec3(handlebarCenter.x, handlebarCenter.y, handlebarCenter.z + dropZ);`
  - `topsEndRight = vec3(handlebarCenter.x, handlebarCenter.y, handlebarCenter.z - dropZ);`
  - `arcRadius = anchors.wheelRadius * 0.30f;`
  - `arcCenter = vec3(topsEndLeft.x, topsEndLeft.y - arcRadius, handlebarCenter.z);`
  - `startAngleDeg = 90.0f;`
  - `endAngleDeg = -140.0f;`
  - `segmentCount = 100;`

## Pedals (RenderPedals)
- Crank arm length and offsets:
  - `crankLength = anchors.wheelRadius * 0.40f;`
  - `crankZOffset = frameParams.tubeRadius * 1.5f;`
  - `leftCrankStart = bottomBracketPos + vec3(0, 0, +crankZOffset);`
  - `rightCrankStart = bottomBracketPos + vec3(0, 0, -crankZOffset);`
  - `leftCrankEnd = leftCrankStart + vec3(0, -crankLength, 0);`
  - `rightCrankEnd = rightCrankStart + vec3(0, +crankLength, 0);`
- Pedal platform sizing:
  - `pedalWidth = anchors.wheelRadius * 0.22f;`
  - `pedalThickness = (frameParams.tubeRadius * 0.35f) * 9.0f;`
  - `pedalHeight = anchors.wheelRadius * 0.02f;`
- Pedal inner-edge offset (keep inner edge fixed):
  - `positionXYZ = crankEnd + vec3(0, 0, +/- (pedalThickness - pedalThicknessBase) * 0.5f);`

## Bicycle Placement (RenderBicycle)
- Wheel radius (fixed):
  - `wheelParams.wheelRadius = 0.75f;`
- Rear wheel center uses bike position offset:
  - `rearWheelCenter = vec3(bikePosition.x, wheelRadius, bikePosition.z);`
- Wheelbase:
  - `bikeWheelbase = wheelRadius * 3.4f;`
  - `frontWheelCenter.x = rearWheelCenter.x + bikeWheelbase;`
