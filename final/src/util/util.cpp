// Creators : Patrick Nagel & Maximilian Meyer


#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251 4275 4290 4996 4231 4244)
#endif

#include <OpenSG/OSGGLUT.h>					//für GLUT
#include <OpenSG/OSGConfig.h>				//für GLUT
#include <OpenSG/OSGGLUTWindow.h>			//für GLUT
#include <OpenSG/OSGSimpleSceneManager.h>	//für GLUT
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGComponentTransform.h>
#include <OpenSG/OSGMaterialGroup.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGSimpleTexturedMaterial.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGTextureBackground.h>
#include <OpenSG/OSGGradientBackground.h>
#include <OpenSG/OSGPointLight.h>
#include <OpenSG/OSGSpotLight.h>
#include <OpenSG/OSGTransitPtr.h>
#include <string> 
#include <ctime>
#include <OpenSG/OSGFieldContainerUtils.h> // für debugging
#include <OpenSG/OSGIntersectAction.h>
#include <OpenSG/OSGVolumeFunctions.h>
#include <iostream>     //for using cout
#include <stdlib.h>     //for using the function sleep
#include <stdio.h>
#include <time.h>
#include <dos.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

OSG_USING_NAMESPACE // activate the OpenSG namespace

NodeRecPtr GhostSpawner(char name[256]){
	NodeRecPtr ghostTrans;
	NodeRecPtr ghostModell = SceneFileHandler::the()->read("models/ghost.3ds");
	ComponentTransformRecPtr ghostCT = ComponentTransform::create();
	ghostTrans = Node::create();
	ghostTrans->setCore(ghostCT);
	
	//create ghost node in tree
	ghostTrans->addChild(ghostModell);

	return ghostTrans;
}