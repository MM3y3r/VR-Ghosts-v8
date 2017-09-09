
//#define SNIPPET_STEP 7

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251 4275 4290 4996 4231 4244)
#endif
// OpenSG includes
#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGSimpleSceneManager.h>
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
#include <ctime>
//test
#ifdef _MSC_VER
# pragma warning(pop)
#endif

OSG_USING_NAMESPACE // activate the OpenSG namespace

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
SimpleSceneManagerRefPtr mgr; // the SimpleSceneManager to manage applications
NodeRecPtr beachTrans;

//------------------------------------------------------------------------------
// Forward Declarations
//------------------------------------------------------------------------------
// forward declaration so we can have the interesting parts upfront
int setupGLUT(int *argc, char *argv[]);

// forward declaration to cleanup the used modules and databases
void cleanup();

NodeTransitPtr createScenegraph() {
	NodeRecPtr root = Node::create();
	root->setCore(Group::create());

	NodeRecPtr boxChild = makeBox(5,4,4,1,1,1);
	NodeRecPtr beach = makePlane(30, 30, 1, 1);

	GeometryRecPtr sunGeo = makeSphereGeo(2, 3);
	NodeRecPtr sunChild = Node::create();
	sunChild->setCore(sunGeo);

	root->addChild(sunChild);
	root->addChild(boxChild);
	root->addChild(beach);

	//decouple the nodes to be shifted in hierarchy from the scene
	root->subChild(sunChild);
	root->subChild(beach);

	TransformRecPtr sunTransCore = Transform::create();
	Matrix sunMatrix;

	// Setting up the matrix
	sunMatrix.setIdentity();
	sunMatrix.setTranslate(0,20,0);
	sunTransCore->setMatrix(sunMatrix); // Adding the Matrix to the core

	// Setting up the node
	NodeRecPtr sunTrans = makeNodeFor(sunTransCore);
	sunTrans->addChild(sunChild);

	ComponentTransformRecPtr ct = ComponentTransform::create();
	ct->setTranslation(Vec3f(0,-2,0));
	ct->setRotation(Quaternion(Vec3f(1,0,0),osgDegree2Rad(90)));

	beachTrans = Node::create();
	beachTrans->setCore(ct);
	beachTrans->addChild(beach);

	// put the nodes in the scene again
	root->addChild(beachTrans);
	root->addChild(sunTrans);
	root->subChild(sunTrans);

	SimpleMaterialRecPtr sunMat = SimpleMaterial::create();
	sunMat->setDiffuse(Color3f(1,0.8f,0));
	sunMat->setAmbient(Color3f(0.8f, 0.2f, 0.2f));

	MaterialGroupRecPtr sunMgCore = MaterialGroup::create();
	sunMgCore->setMaterial(sunMat);

	NodeRecPtr sunMg = Node::create();

	sunMg->setCore(sunMgCore);
	sunMg->addChild(sunTrans);

	root->addChild(sunMg);

	SimpleMaterialRecPtr boxMat = SimpleMaterial::create();

	boxMat->setDiffuse(Color3f(1,0.2f,0.1f));
	boxMat->setAmbient(Color3f(0.8f, 0.2f, 0.2f));
	boxMat->setTransparency(0.25);
	//boxMat->setLit(false);

	GeometryRecPtr boxGeo = dynamic_cast<Geometry*>(boxChild->getCore());
	boxGeo->setMaterial(boxMat);
	ImageRecPtr image = Image::create();
	// sand taken from http://www.filterforge.com/filters/720.jpg
	image->read("models/sand.jpg");

	//now we create the texture that will hold the image
	SimpleTexturedMaterialRecPtr tex = SimpleTexturedMaterial::create();
	tex->setImage(image);

	//now assign the fresh texture to the geometry
	GeometryRecPtr beachGeo = dynamic_cast<Geometry*>(beach->getCore());
	beachGeo->setMaterial(tex);

	//model taken from http://storage3d.com/
	NodeRecPtr palmTree = SceneFileHandler::the()->read("models/palm.3ds");
	

	ComponentTransformRecPtr palmCT = ComponentTransform::create();
	palmCT->setTranslation(Vec3f(12,-1,0));
	palmCT->setRotation(Quaternion(Vec3f(0,1,0),osgDegree2Rad(45)));
	palmCT->setScale(Vec3f(10.f,10.f,10.f));

	NodeRecPtr palmTrans = makeNodeFor(palmCT);
	palmTrans->addChild(palmTree);

	root->addChild(palmTrans);

	NodeRecPtr palmTree2 = OSG::deepCloneTree(palmTrans);
	ComponentTransformRecPtr palmCT2 = dynamic_cast<ComponentTransform*>(palmTree2->getCore());

	palmCT2->setTranslation(Vec3f(10,-1,5));
	palmCT->setRotation(Quaternion(Vec3f(1,0,0),osgDegree2Rad(0)));
	palmCT->setScale(Vec3f(10.f,10.f,10.f));

	NodeRecPtr palmTree3 = OSG::deepCloneTree(palmTrans);
	ComponentTransformRecPtr palmCT3 = dynamic_cast<ComponentTransform*>(palmTree3->getCore());

	palmCT3->setTranslation(Vec3f(20,-1,5));

	root->addChild(palmTree2);
	root->addChild(palmTree3);

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
	return NodeTransitPtr(root);
}

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

		NodeRecPtr root = createScenegraph();

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

		// ICH BIN EIN TEST

		gwin->getPort(0)->setBackground(imBkg);
	}
	glutMainLoop();
}

void display() {
	// TEST CHANGE
	const float time = 1000.f * std::clock() / CLOCKS_PER_SEC;

	ComponentTransformRecPtr bt = dynamic_cast<ComponentTransform*>(beachTrans->getCore());

	//bt->setTranslation(Vec3f(10,5,0));
	bt->setRotation(Quaternion(Vec3f(1,0,0),osgDegree2Rad(270)+0.001f*time));
	//bt->setScale(Vec3f(0.001,0.001,0.001));

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
	default:
		break;
	}
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
	beachTrans = NULL;
	mgr = NULL;
}
