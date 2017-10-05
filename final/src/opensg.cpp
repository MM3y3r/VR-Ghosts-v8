// Creators : Patrick Nagel & Maximilian Meyer

// file includes
#include "./util/pointController.h"
#include "./util/AABB.h"
#include "./util/util.h"

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


#ifdef _MSC_VER
# pragma warning(pop)
#endif

OSG_USING_NAMESPACE // activate the OpenSG namespace



//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
SimpleSceneManagerRefPtr mgr; // the SimpleSceneManager to manage applications
NodeRecPtr ghostTrans;
ComponentTransformRecPtr heartTransCore;
NodeRecPtr heartTrans;
NodeRecPtr heartTrans1;
NodeRecPtr heartTrans2;
NodeRecPtr heartTrans3;
NodeRecPtr heartTrans4;

NodeRecPtr MatGroupNode;

static int heartCounter = 0;
SimpleMaterialRecPtr heartMaterial;


UInt8 mode = 0; //change the mode of our game
NodeRecPtr root;
int ghostCount = 0; // counts how many ghosts we have got in the scene

//------------------------------------------------------------------------------
// Forward Declarations
//------------------------------------------------------------------------------

int setupGLUT(int *argc, char *argv[]);

// forward declaration to cleanup the used modules and databases
void cleanup();

class Heart {

	static int nextId() 
	{
		static int lastId = 0;
		return ++lastId;
	}

public:
	//int direction;
	int heartId;
	const ComponentTransformRecPtr trans;

	Heart(const ComponentTransformRecPtr trans, int heartId)
		: heartId(nextId()), trans(trans) {
	}
	
	ComponentTransformRecPtr getTrans(){
		return this->trans;
	}
	//NodeTransitPtr setTransitPointer(NodeTransitPtr transitPointer){
	//	this->ghostTrans = transitPointer;
	//}
};

std::vector<std::pair<Heart,NodeRecPtr> > hearts;


NodeRecPtr heartFactory(int heartId){
	
	static NodeRecPtr heartModell = makeBox(1,1,1,1,1,1);
	heartTransCore = ComponentTransform :: create();
	
	// Material for the heart
	heartMaterial = SimpleMaterial::create();
	heartMaterial->setDiffuse(Color3f(1,0,0));
	heartMaterial->setAmbient(Color3f(0.2,0.2,0.2));
	heartMaterial->setTransparency(0.5);
	
	// Material Node
	MaterialGroupRecPtr heartMatGroup = MaterialGroup::create();
	heartMatGroup->setMaterial(heartMaterial);
	MatGroupNode = Node::create();
	MatGroupNode->setCore(heartMatGroup);

	ComponentTransformRecPtr heartCT = ComponentTransform::create();
	NodeRecPtr heartTrans = makeNodeFor(heartCT);

	// deepCloning the heart pixelö
	heartTrans->addChild(OSG::deepCloneTree(heartModell));
	MatGroupNode->addChild(heartTrans);
	heartTrans->setCore(heartTransCore);

	hearts.push_back(std::pair<Heart,NodeRecPtr>(Heart(heartTransCore,heartId),MatGroupNode));
	//ghosts.push_back(std::pair<Ghost,NodeRecPtr>(Ghost(ghostCT,startTime,ghostCounter),ghostTrans));

	return NodeRecPtr(MatGroupNode); // in transit gepackt
}

NodeTransitPtr createScenegraph() {
	// CREATE ROOT SCENE GRAPH
	NodeRecPtr root = Node::create(); //Generation of objects via create() generates the object internally and returns a pointer to the object (RecPtr)
	root->setCore(Group::create()); // A first node with a group core is created - this is achieved by creating an anonymous group core and setting it in the node

	//• NodeRecPtr		-> General smart pointer used to create a node
	//• NodeTransitPtr	-> Used for passing a node pointer out of the current context, typically used when a field container is created inside a function and should be returned
	//• commitChanges	-> Commit the changes before rendering, fields become synchronise over all rendering nodes
	//• Attachment		-> Field containers can have attachments (user data) Typically available with nodes and node cores Attachments have to be derived from the class attachment

	// CREATE SCENE BOX 
	// (Hier wurden zwei Möglichkeiten präsentiert ->	1.makeirgendwas (we create a node containing a geometry core representing a box as well as a node containing a geometry)
	//													2.makeirgendwasGeo (we create a geometry core defining a sphere and an additional empty node To fill the empty node we have to set the geometry core inside this nodecore representing a plane)

	for (int i = 0; i < 28; i++)
	{
		root->addChild(heartFactory(i));
	}


	//heartTrans1 = Node::create();
	//heartTrans2 = Node::create();
	//heartTrans3 = Node::create();
	//heartTrans4 = Node::create();
	//heartTransCore = Transform :: create();
	//
	//NodeRecPtr heart1 = makeBox(1,1,1,1,1,1);
	//NodeRecPtr heart2 = makeBox(1,1,1,1,1,1);
	//NodeRecPtr heart3 = makeBox(1,1,1,1,1,1);
	//NodeRecPtr heart4 = makeBox(1,1,1,1,1,1);
	//root->addChild(heart1);
	//root->addChild(heart2);
	//root->addChild(heart3);
	//root->addChild(heart4);

	//// Material for the heart
	//heartMaterial = SimpleMaterial::create();
	//heartMaterial->setDiffuse(Color3f(1,0,0));
	//heartMaterial->setAmbient(Color3f(0.2,0.2,0.2));
	//heartMaterial->setTransparency(0.5);

	//MaterialGroupRecPtr heartMatGroup = MaterialGroup::create();
	//heartMatGroup->setMaterial(heartMaterial);
	//NodeRecPtr MatGroupNode = Node::create();
	//MatGroupNode->setCore(heartMatGroup);

	//MatGroupNode->addChild(heartTrans1);
	//MatGroupNode->addChild(heartTrans2);
	//MatGroupNode->addChild(heartTrans3);
	//MatGroupNode->addChild(heartTrans4);
	//heartTrans1->setCore(heartTransCore);
	//heartTrans2->setCore(heartTransCore);
	//heartTrans3->setCore(heartTransCore);
	//heartTrans4->setCore(heartTransCore);


	//heartTrans1->addChild(heart1);
	//heartTrans2->addChild(heart2);
	//heartTrans3->addChild(heart3);
	//heartTrans4->addChild(heart4);

	//MatGroupNode->addChild(heartTrans1);
	//MatGroupNode->addChild(heartTrans2);
	//MatGroupNode->addChild(heartTrans3);
	//MatGroupNode->addChild(heartTrans4);


	//root->addChild(MatGroupNode);


	//NodeRecPtr boxChild2 = makeBox(5,5,5,1,1,1);

	GeometryRecPtr sunGeo = makeSphereGeo(2, 3);	//	2.Only Core (GEO)
	NodeRecPtr sunChild = Node::create();			//	2.empty node
	sunChild->setCore(sunGeo);						//	2.placing core in node

	root->addChild(sunChild);						//	Die drei Nodes werden an die Root gebunden
	
	//root->addChild(boxChild2);						

	//decouple the nodes to be shifted in hierarchy from the scene
	root->subChild(sunChild); //remove


	//sun transformation
	TransformRecPtr sunTransCore = Transform::create();
	Matrix sunMatrix;

	// Setting up the matrix
	sunMatrix.setIdentity();
	sunMatrix.setTranslate(0,20,0);
	sunTransCore->setMatrix(sunMatrix); // Adding the Matrix to the core (The matrix is set in the core, the core is set in a node, and all the child nodes are transformed the way described in the matrix)

	// Setting up the node
	NodeRecPtr sunTrans = makeNodeFor(sunTransCore);
	sunTrans->addChild(sunChild);

	root->addChild(sunTrans);
	root->subChild(sunTrans);

	// Hier: material erstellen, kern machen, knoten erstellen, knoten kern geben, knoten child geben
	SimpleMaterialRecPtr sunMat = SimpleMaterial::create();
	sunMat->setDiffuse(Color3f(1,0.8f,0));
	sunMat->setAmbient(Color3f(0.8f, 0.2f, 0.2f));

	MaterialGroupRecPtr sunMgCore = MaterialGroup::create();
	sunMgCore->setMaterial(sunMat);

	NodeRecPtr sunMg = Node::create();

	sunMg->setCore(sunMgCore);
	sunMg->addChild(sunTrans);
	//sunMg->addChild(utrans);

	root->addChild(sunMg);

	//-----------------------------------------------------
	//KOLLISIONSBOX-TRANSFORMATION (evtl. deprecated)
	//-----------------------------------------------------


	


	//-----------------------------------------------------
	// Ghost Modell & Transform
	//-----------------------------------------------------
	
	NodeRecPtr ghostModell = SceneFileHandler::the()->read("models/ghost.3ds");
	ComponentTransformRecPtr ghostCT = ComponentTransform::create();
	ghostTrans = Node::create();
	ghostTrans->setCore(ghostCT);
	
	//create ghost node in tree
	ghostTrans->addChild(ghostModell);

	//Add our Ghost to the root
	root->addChild(ghostTrans);
	
	//-----------------------------------------------------
	//Colission detection -versuch
	//-----------------------------------------------------

	const float time = 1000.f * std::clock() / CLOCKS_PER_SEC; 


	PointLightRecPtr sunLight = PointLight::create();
	//sunLight->setAttenuation(1,0,2);

	//color information
	sunLight->setDiffuse(Color4f(1,1,1,1));
	sunLight->setAmbient(Color4f(0.2f,0.2f,0.2f,1));
	sunLight->setSpecular(Color4f(1,1,1,1));

	sunLight->setBeacon(sunChild); //attach to the sun node use this node as position beacon

	root->setCore(sunLight);

	DirectionalLightRecPtr dirLight = DirectionalLight::create();
	dirLight->setDirection(1,1,-1);

	//color information
	dirLight->setDiffuse(Color4f(1,1,1,1));
	dirLight->setAmbient(Color4f(0.2f,0.2f,0.2f,1));
	dirLight->setSpecular(Color4f(1,1,1,1));


	//wrap the root, cause only nodes below the lights will be lit
	NodeRecPtr ueberroot = makeNodeFor(dirLight);
	ueberroot->addChild(root);

	root = ueberroot;
	return NodeTransitPtr(root); // We need such a transit pointer to pass the node pointer from the scene graph out of the function to main
}

//This is the function that will be called when a node
//is entered during traversal.
Action::ResultE enter(Node* node){
	if(node->getCore()->getType().isDerivedFrom(ComponentTransform::getClassType()))
		std::cout << "Enter node : " << node << std::endl;

	return Action::Continue;
}

Action::ResultE leave(Node* node, Action::ResultE result){
//  std::cout << "Leaving node: " << node << "with code: " << result << std::endl;
	return result;
}
int main(int argc, char **argv) {


	// EXAMPLES
	addToScore(5);
	removeFromScore(3);

#if WIN32
	OSG::preloadSharedObject("OSGFileIO");
	OSG::preloadSharedObject("OSGImageFileIO");
#endif
	{
		osgInit(argc, argv); // initialize OpenSG

		int winid = setupGLUT(&argc, argv);  // initialize GLUT

		// the connection between GLUT and OpenSG is established
		GLUTWindowRecPtr gwin = GLUTWindow::create();
		gwin->setGlutId(winid);
		gwin->init();

		root = createScenegraph();

		traverse(root,enter,leave);


		mgr = OSG::SimpleSceneManager::create();
		mgr->setWindow(gwin);			// tell the manager what to manage
		mgr->setRoot(root);				// attach the scenegraph
		mgr->setHeadlight(false);
		mgr->showAll();					// show the whole scene

		ImageRecPtr backimage = Image::create();
		backimage->read("models/mansion1.jpg");

		TextureObjChunkRecPtr bkgTex = TextureObjChunk::create();
		bkgTex->setImage(backimage);
		bkgTex->setScale(false);
		TextureBackgroundRecPtr imBkg = TextureBackground::create();
		imBkg->setTexture(bkgTex);
		imBkg->setColor(Color4f(1.0,1.0,1.0,0.0f));

		// alternatively use a gradient background
		//GradientBackgroundRecPtr bkg = GradientBackground::create();
		//bkg->addLine(Color3f(0.7f, 0.7f, 0.8f), 0);
		//bkg->addLine(Color3f(0.0f, 0.1f, 0.3f), 1);

		gwin->getPort(0)->setBackground(imBkg);
	}
	glutMainLoop();
}


void display() {

	const float time = 1000.f * std::clock() / CLOCKS_PER_SEC;

	for (auto &heart : hearts) {
		switch(heart.first.heartId){
			// heart lvl 1
			case (1):
				heart.first.trans->setTranslation(Vec3f(0,0,0));
				break;
			// heart lvl 2
			case (2):
				heart.first.trans->setTranslation(Vec3f(-1,1,0));
				break;
			case (3):
				heart.first.trans->setTranslation(Vec3f(0,1,0));
				break;
			case (4):
				heart.first.trans->setTranslation(Vec3f(1,1,0));
				break;
			// heart lvl 3
			case (5):
				heart.first.trans->setTranslation(Vec3f(-2,2,0));
				break;
			case (6):
				heart.first.trans->setTranslation(Vec3f(-1,2,0));
				break;
			case (7):
				heart.first.trans->setTranslation(Vec3f(0,2,0));
				break;
			case (8):
				heart.first.trans->setTranslation(Vec3f(1,2,0));
				break;
			case (9):
				heart.first.trans->setTranslation(Vec3f(2,2,0));
				break;
			// heart lvl 4
			case (10):
				heart.first.trans->setTranslation(Vec3f(-3,3,0));
				break;
			case (11):
				heart.first.trans->setTranslation(Vec3f(-2,3,0));
				break;
			case (12):
				heart.first.trans->setTranslation(Vec3f(-1,3,0));
				break;
			case (13):
				heart.first.trans->setTranslation(Vec3f(0,3,0));
				break;
			case (14):
				heart.first.trans->setTranslation(Vec3f(1,3,0));
				break;
			case (15):
				heart.first.trans->setTranslation(Vec3f(2,3,0));
				break;
			case (16):
				heart.first.trans->setTranslation(Vec3f(3,3,0));
				break;
			// heart lvl 5
			case (17):
				heart.first.trans->setTranslation(Vec3f(-3,4,0));
				break;
			case (18):
				heart.first.trans->setTranslation(Vec3f(-2,4,0));
				break;
			case (19):
				heart.first.trans->setTranslation(Vec3f(-1,4,0));
				break;
			case (20):
				heart.first.trans->setTranslation(Vec3f(0,4,0));
				break;
			case (21):
				heart.first.trans->setTranslation(Vec3f(1,4,0));
				break;
			case (22):
				heart.first.trans->setTranslation(Vec3f(2,4,0));
				break;
			case (23):
				heart.first.trans->setTranslation(Vec3f(3,4,0));
				break;
			// heart lvl 6 (TOP)
			case (24):
				heart.first.trans->setTranslation(Vec3f(-2,5,0));
				break;
			case (25):
				heart.first.trans->setTranslation(Vec3f(-1,5,0));
				break;
			case (26):
				heart.first.trans->setTranslation(Vec3f(1,5,0));
				break;
			case (27):
				heart.first.trans->setTranslation(Vec3f(2,5,0));
				break;
		}
		
	}

	// definition time


	//ComponentTransformRecPtr bt = dynamic_cast<ComponentTransform*>(beachTrans->getCore());
	//ComponentTransformRecPtr zt = dynamic_cast<ComponentTransform*>(trans->getCore());

	// -----------------UNSERE GHOST BEWEGUNG -------------------------
	ComponentTransformRecPtr ghostDC = dynamic_cast<ComponentTransform*>(ghostTrans->getCore());
	ghostDC->setTranslation(Vec3f(0,0,0.001f*time));
    
	//ComponentTransformRecPtr heartDC1 = dynamic_cast<ComponentTransform*>(heartTrans1->getCore());
	//ComponentTransformRecPtr heartDC2 = dynamic_cast<ComponentTransform*>(heartTrans2->getCore());
	//ComponentTransformRecPtr heartDC3 = dynamic_cast<ComponentTransform*>(heartTrans3->getCore());
	//ComponentTransformRecPtr heartDC4 = dynamic_cast<ComponentTransform*>(heartTrans4->getCore());
	//heartDC1->setTranslation(Vec3f(0,0,0));
	//heartDC2->setTranslation(Vec3f(1,1,0));
	//heartDC3->setTranslation(Vec3f(-1,1,0));
	//heartDC4->setTranslation(Vec3f(0,2,0));

    // ----------------- GHOST SINUS BEWEGUNG -------------------------
    // update the object's position based on its velocity:
    // pos += velocity;
    
    // adjust the velocity by adding or subtracting a fixed value
    //   we'll call this value 'foobar' because I'm too lazy to come up
    //   with a better name.
    //
    // 'centerline' is the Y position which will be the center point of the
    //   sine wave
    
    /* TODO:
     if( pos.y < centerline )
        velocity.y += foobar;
    else
        velocity.y -= foobar;
     */
    // ----------------- END GHOST SINUS BEWEGUNG -------------------------
    
	//bt->setTranslation(Vec3f(10,5,0));
	//bt->setRotation(Quaternion(Vec3f(1,0,0),osgDegree2Rad(270)+0.001f*time));
	//bt->setScale(Vec3f(0.001,0.001,0.001));

	//zt->setTranslation(Vec3f(10,5,0.001f*time));
	//zt->setRotation(Quaternion(Vec3f(1,0,0),osgDegree2Rad(270)+0.005f*time));
	//zt->setScale(Vec3f(0.001,0.001,0.001));

	//BASIC LOOP
	//for (int n=10; n>0; n--) {
	//	printf( "starting to sleep...\n" );
	//	Sleep( 3000 );   // sleep three seconds
	//	printf( "sleep ended\n" );
	//}




	//updateMesh(time);
	commitChanges(); //make sure the changes are distributed over the containers
	mgr->redraw(); // redraw the window
}

void reshape(int w, int h) {
	mgr->resize(w, h); // react to size changes
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	// react to mouse button presses
	if (state) {
		mgr->mouseButtonRelease(button, x, y);
	} else {
		mgr->mouseButtonPress(button, x, y);

		// on mouse button press create ray/line
		Line ray = mgr->calcViewRay(x, y);

		//create intersection action
		IntersectActionRefPtr iAct = IntersectAction::create();
		iAct->setLine(ray);
		iAct->apply(root);

		if (iAct->didHit()){
			//std::cout << "Hit Point : "<< iAct->getHitPoint();

			//get the hit point
			Pnt3f p = iAct->getHitPoint();
			//std::cout << "Hit point : " << p[0] << " " << p[1] << " " << p[2] << std::endl;

			//and the node that was hit
			NodeRefPtr n = iAct->getHitObject();
			//std::cout << "Hit Object : "<< iAct->getHitObject();

			// remove node from scene
			NodeRefPtr parent = n->getParent();
			parent->subChild(n);
		}
	}
	glutPostRedisplay();
}

void motion(int x, int y) {
	mgr->mouseMove(x, y);
	glutPostRedisplay();
}

void keyboard(unsigned char k, int x, int y) {
	switch (k) {
	case 27: // 27 = esc-button
		cleanup();
		exit(0);
		break;
		case 'w' : mode = 0; break;
		case 'a' : mode = 1; break;
		case 's' : mode = 2; break;
		case 'd' : mode = 3; break;
	default:
		break;
	}
	std::string modeString = std::to_string(mode);
	std::cout << "entering mode: " << modeString + "\n";
}

int setupGLUT(int *argc, char *argv[]) {
	// setup the GLUT library which handles the windows for us
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(100,100);

	int winid = glutCreateWindow("Virtual Reality Lab: OpenSG");

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
//	glutPassiveMotionFunc(motion);
	glutIdleFunc(display);

	return winid;
}


void cleanup() {
	//beachTrans = NULL;
	//trans = NULL;
	ghostTrans = NULL;
	heartTrans1= NULL;
	mgr = NULL;
}
