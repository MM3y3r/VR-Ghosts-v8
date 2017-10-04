#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <math.h>       /* sin */
#include <iostream>
#include <ios>
#include <vector>


#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGMultiDisplayWindow.h>
#include <OpenSG/OSGSceneFileHandler.h>
//#include <OpenSG/OSGSimpleAttachement.h>

#include <OSGCSM/OSGCAVESceneManager.h>
#include <OSGCSM/OSGCAVEConfig.h>
#include <OSGCSM/appctrl.h>

#include <vrpn_Tracker.h>
#include <vrpn_Button.h>
#include <vrpn_Analog.h>


#include <OpenSG/OSGMaterialGroup.h>
#include <OpenSG/OSGMatrix.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGSimpleTexturedMaterial.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <OpenSG/OSGIntersectAction.h>
#include <OpenSG/OSGComponentTransformBase.h>
#include <string>

using namespace std;

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------


#define _USE_MATH_DEFINES

OSG_USING_NAMESPACE

OSGCSM::CAVEConfig cfg;
OSGCSM::CAVESceneManager *mgr = nullptr;
vrpn_Tracker_Remote* tracker = nullptr;
vrpn_Button_Remote* button = nullptr;
vrpn_Analog_Remote* analog = nullptr;

NodeRecPtr root = nullptr;

static int ghostCounter = 0;

NodeRecPtr coneTrans;
TransformRecPtr coneTransCore;



NodeRecPtr flashLightCone;
SimpleMaterialRecPtr flashLightMaterial;



//coneTrans->addChild(flashLightCone);
//root->addChild(coneTrans);

//------------------------------------------------------------------------------
// Cleanup
//------------------------------------------------------------------------------

void cleanup()
{
	delete mgr;
	delete tracker;
	delete button;
	delete analog;
}

void print_tracker();

//------------------------------------------------------------------------------
// Ghost Class with functionality
//------------------------------------------------------------------------------

class Ghost {

	static int nextId() 
	{
	static int lastId = 0;
	return ++lastId;
	}

public:
	//int direction;
	int ghostId;
	const ComponentTransformRecPtr trans;
	OSG::Time startTime;

	Ghost(const ComponentTransformRecPtr trans, OSG::Time startTime, int ghostId)
		: ghostId(nextId()), trans(trans), startTime(startTime) {
	}
	
	ComponentTransformRecPtr getTrans(){
		return this->trans;
	}
	//NodeTransitPtr setTransitPointer(NodeTransitPtr transitPointer){
	//	this->ghostTrans = transitPointer;
	//}
};

std::vector<std::pair<Ghost,NodeRecPtr> > ghosts; // habe auf NodeRecprt ge�ndert f�r die second funktion

//------------------------------------------------------------------------------
// Ghost Factory
//------------------------------------------------------------------------------

//int nextId() {
//	++ghostCounter;
//	//std::cout << "ghostCounter @countup : " << ghostCounter << '\n';
//	return ghostCounter;
//};
//
//void countUp(){
//	ghostCounter = ghostCounter+1;
//	//std::cout << "ghostCounter @countup : "<< ghostCounter << '\n';
//}

NodeTransitPtr ghostFactory(OSG::Time startTime){


	static NodeRecPtr ghostModell = SceneFileHandler::the()->read("./models/BO.obj");
	

	ComponentTransformRecPtr ghostCT = ComponentTransform::create();
	ComponentTransformRecPtr emptyCore = ComponentTransform::create();
	

	NodeRecPtr ghostTrans = makeNodeFor(ghostCT);
	NodeRecPtr bumpNode = makeNodeFor(emptyCore); // auf rec
	//Ghost tmpGhost(ghostCT,startTime, nextId());
	

	// deepCloning the ghostModell
	bumpNode->addChild(OSG::deepCloneTree(ghostModell));
	ghostTrans->addChild(bumpNode);
	//ghosts.push_back(std::pair<Ghost,NodeTransitPtr>(Ghost(ghostCT,startTime,ghostCounter),NodeTransitPtr(ghostTrans))); // l�uft so
	ghosts.push_back(std::pair<Ghost,NodeRecPtr>(Ghost(ghostCT,startTime,ghostCounter),ghostTrans));

	return NodeTransitPtr(ghostTrans); // in transit gepackt
}

//------------------------------------------------------------------------------
// Build the scene
//------------------------------------------------------------------------------

NodeTransitPtr buildScene()
{
	root = makeNodeFor(Group::create());
	//Add our Ghost to the root
	root->addChild(ghostFactory(0));

	coneTrans = Node::create();
	coneTransCore = Transform :: create();

	// Material for the flashlight
	flashLightMaterial = SimpleMaterial::create();
	flashLightMaterial->setDiffuse(Color3f(1,1,0));
	flashLightMaterial->setAmbient(Color3f(0.2,0.2,0.2));
	flashLightMaterial->setTransparency(0.5);

	MaterialGroupRecPtr coneMatGroup = MaterialGroup::create();
	coneMatGroup->setMaterial(flashLightMaterial);
	NodeRecPtr MatGroupNode = Node::create();
	MatGroupNode->setCore(coneMatGroup);
	MatGroupNode->addChild(coneTrans);

	coneTrans->setCore(coneTransCore);

	flashLightCone = SceneFileHandler::the()->read("./models/cone.obj");
	//flashLightCone = makeCylinder(1000,1,4,true,true,true);
	//const CylinderVolume &vol = flashLightCone->getVolume(false);
	//makeBox(20,20,20,10,10,10);

		//vol.setEmpty();
	//flashLightCone->invalidateVolume();

	//coneTrans->addChild(falsLightMaterial);

	coneTrans->addChild(flashLightCone);
	MatGroupNode->addChild(coneTrans);

	root->addChild(MatGroupNode);

	return NodeTransitPtr(root);
}

//------------------------------------------------------------------------------
// Tracker and Wand --- TODO: Wand = Flashlight
//------------------------------------------------------------------------------

//template<typename T>
T scale_tracker2cm(const T& value)
{
	static const float scale = OSGCSM::convert_length(cfg.getUnits(), 1.f, OSGCSM::CAVEConfig::CAVEUnitCentimeters);
	return value * scale;
}

auto head_orientation = Quaternion(Vec3f(0.f, 1.f, 0.f), 3.141f);
auto head_position = Vec3f(0.f, 170.f, 200.f);	// a 1.7m Person 2m in front of the scene

void VRPN_CALLBACK callback_head_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	head_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	head_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
}

auto wand_orientation = Quaternion();
auto wand_orientation_flashLight = Quaternion();
auto wand_position = Vec3f();
Matrix4f wandRotationMat;
Matrix4f wandHitMat;

Matrix4f rotMat = Matrix(1,0,0,0,0,cos(90),-sin(90),0,0,sin(90),cos(90),0,0,0,0,1); // für cone

void VRPN_CALLBACK callback_wand_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	wand_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	wand_orientation_flashLight = Quaternion(tracker.quat[0] + osgDegree2Rad(90), tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	wand_position = Vec3f(Vec3d(tracker.pos));
	//std::cout << " quat 0 : "<< tracker.quat[0] << '\n';
	
}

auto analog_values = Vec3f();
void VRPN_CALLBACK callback_analog(void* userData, const vrpn_ANALOGCB analog)
{
	if (analog.num_channel >= 2)
		analog_values = Vec3f(analog.channel[0], 0, -analog.channel[1]);
}







void VRPN_CALLBACK callback_button(void* userData, const vrpn_BUTTONCB button)
{
	if (button.button == 0 && button.state == 1){

		//------------------------------------------------------------------------------
		// Intersection with target
		//------------------------------------^^------------------------------------------

		// on wand button press create ray/line

					//Line ray;
		Vec4f lineDir(0.,0.,-1.,0.);
						/*Pnt3f lineStartingPoint = Vec3f(wand_position.x(), wand_position.y()+10, wand_position.z()+10);
						lineDir = wandHitMat * lineDir;*/
		Vec4f lineStartingPoint = Vec4f(wand_position.x(), wand_position.y(), wand_position.z(), 0); // auf ve4f erweitert
						lineDir = wandHitMat * lineDir; // quaternion + quaternion


		Vec3f tempChekPoint(0,0,0);
		int count = 0;
		int maxCount = 1000;
		while (count < maxCount)
		{
			count++; // AHHHHHH WICHTIG
			////int lineDirInt = lineDir.normalize;
			//
			lineDir.normalize();
			//
			//lineDir.normalize()*(count);
			//
			//

			////lineDir.normalize;
			//
			//tempChekPoint.setValue((lineDir*count)+lineStartingPoint);

			//lineStartingPoint.operator+(lineDir);
			tempChekPoint.setValue(lineStartingPoint.operator+(lineDir*count));
			//tempChekPoint.setValue(lineStartingPoint.operator+(lineDir*count));
			//tempChekPoint.setValue(tempChekPoint+lineStartingPoint);

			//tempChekPoint.setValue(lineStartingPoint+(lineDir*count)); //Pnt3f + quaternion nomalized*int

			

			for (auto ghost : ghosts)
				//const auto time = timeSinceStart - ghost.first.startTime;

			{
				Vec3f position (0,0,0);
				position.setValue(ghost.first.trans->getTranslation());
				Vec3f betrag (position.operator-(tempChekPoint));

				if (betrag.length() < 15)
				{
					//root->subChild(ghost.second);
					std::cout << "Hit: " << '\n';
					break;
				}
			};
			
		}




		//lineStartingPoint.setValues(wand_orientation[0], wand_orientation[1], wand_orientation[2]);
					//ray.setValue(lineStartingPoint, Vec3f(lineDir.x(),lineDir.y(),lineDir.z()));
		//std::cout << "Resulting line : "<< ray  << '\n';

		//std::cout << "Wand position: " << lineStartingPoint << " orientation: " << wand_orientation << '\n';

		//wand_position.getValues();
		//ray.setValue(wand_position.getValues(), wand_orientation);

		//create intersection action
	//	IntersectActionRefPtr iAct = IntersectAction::create();
	//	iAct->setLine(ray, 10000);
	//	iAct->apply(root);

	//	if (iAct->didHit()){
	//		std::cout << "Hit Point : "<< iAct->getHitPoint() << '\n';

	//		//get the hit point
	//		Pnt3f p = iAct->getHitPoint();
	//		std::cout << "Hit point : " << p[0] << " " << p[1] << " " << p[2] << std::endl << '\n';

	//		//and the node that was hit
	//		NodeRefPtr nodeHit = iAct->getHitObject();
	//		std::cout << "Hit Object : "<< iAct->getHitObject() << '\n';

	//		// remove node from scene
	//		// LOOOL
	//		NodeRefPtr ghostParent = nodeHit->getParent()->getParent()->getParent();
	//		root->subChild(ghostParent);
	//	}

	//	// print out coords
	//	print_tracker();
	//}
}
}

void InitTracker(OSGCSM::CAVEConfig &cfg)
{
	try
	{
		const char* const vrpn_name = "DTrack@localhost";
		tracker = new vrpn_Tracker_Remote(vrpn_name);
		tracker->shutup = true;
		tracker->register_change_handler(NULL, callback_head_tracker, cfg.getSensorIDHead());
		tracker->register_change_handler(NULL, callback_wand_tracker, cfg.getSensorIDController());
		button = new vrpn_Button_Remote(vrpn_name);
		button->shutup = true;
		button->register_change_handler(nullptr, callback_button);
		analog = new vrpn_Analog_Remote(vrpn_name);
		analog->shutup = true;
		analog->register_change_handler(NULL, callback_analog);
	}
	catch(const std::exception& e) 
	{
		std::cout << "ERROR: " << e.what() << '\n';
		return;
	}
}

void check_tracker()
{
	tracker->mainloop();
	button->mainloop();
	analog->mainloop();
}

void print_tracker()
{
	//std::cout << "Head position: " << head_position << " orientation: " << head_orientation << '\n';
	std::cout << "Wand position: " << wand_position[0] << " orientation: " << wand_orientation << '\n';
	//std::cout << "Analog: " << analog_values << '\n';
}

void keyboard(unsigned char k, int x, int y)
{
	Vec3f test (6,3,6);
	Vec3f test2 (0,0,0);
	Real32 ed;
	switch(k)
	{
		case 'q':
		case 27: 
			cleanup();
			exit(EXIT_SUCCESS);
			break;
		case 'e':
			ed = mgr->getEyeSeparation() * .9f;
			std::cout << "Eye distance: " << ed << '\n';
			mgr->setEyeSeparation(ed);
			break;
		case 'E':
			ed = mgr->getEyeSeparation() * 1.1f;
			std::cout << "Eye distance: " << ed << '\n';
			mgr->setEyeSeparation(ed);
			break;
		case 'h':
			cfg.setFollowHead(!cfg.getFollowHead());
			std::cout << "following head: " << std::boolalpha << cfg.getFollowHead() << '\n';
			break;
		case 'i':
			print_tracker();
			break;
		case 't':
			test.normalize();
			
			std::cout << "following head: " << test*2 << '\n';
		break;
		default:
			std::cout << "Key '" << k << "' ignored\n";
	}
}

//------------------------------------------------------------------------------
// setup glut with display function
//------------------------------------------------------------------------------

//OSG::Vec3f

void setupGLUT(int *argc, char *argv[])
{
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGB  |GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("OpenSG CSMDemo with VRPN API");
	glutDisplayFunc([]()
	{

		// ----------------- WAND HANDLING -------------------------------------------

		wandRotationMat.setTransform(wand_orientation);
		wandHitMat.setTransform(wand_orientation);
		wandRotationMat.mult(rotMat);
		wandRotationMat.setTranslate(wand_position);
		wandHitMat.setTranslate(wand_position);
		coneTransCore->setMatrix(wandRotationMat);
		// ---------------------------------------------------------------------------

		// black navigation window
		glClear(GL_COLOR_BUFFER_BIT);
		glutSwapBuffers();

		static OSG::Time startTime = OSG::getSystemTime();
		OSG::Time currentTime = OSG::getSystemTime();
		OSG::Time timeSinceStart = currentTime - startTime;

		// ----------------- SINE CALC ------------------------------------------------
		double sineWave;
		double sineWave2;
		double sineFactor = 30;
		double sineFactor2 = 50;
		sineWave = sin (120*timeSinceStart*3.14/180) * sineFactor;
		sineWave2 = sin (80*timeSinceStart*3.14/180) * sineFactor2;
		//printf ("The sine Nr 1. of %f degrees is %f.\n", timeSinceStart, sineWave );
		//printf ("The sine Nr 2. of %f degrees is %f.\n", timeSinceStart, sineWave );
		//------------------------------------------------------------------------------

		// ----------------- GHOST SPEED -----------------------------------------------
		double ghostSpeed = 10;
		//------------------------------------------------------------------------------

		check_tracker();
		const auto speed = 1.f;
		mgr->setUserTransform(head_position, head_orientation);
		//mgr->setTranslation(mgr->getTranslation() + speed * analog_values);

		// ----------------- GHOST MOVEMENT --------------------------------------------
		static OSG::Time nextGhostSpawnTime = 10;
		if (timeSinceStart > nextGhostSpawnTime) {
			root->addChild(ghostFactory(timeSinceStart));
			nextGhostSpawnTime += 10;
		}

		//std::cout << "before the loop " << '\n';

		for (auto &ghost : ghosts) {
			const auto time = timeSinceStart - ghost.first.startTime;
			//std::cout << "forschleife \n " << ghost.first.getTrans() << '\n';
			//std::cout << "Ghost ID in der vorschleife  \n" << ghost.first.ghostId << '\n';
			switch (ghost.first.ghostId % 4)
			{
				// VORNE
				case (0):
					//std::cout << "case 0 : " << ghost.first.trans << '\n';
					ghost.first.trans->setTranslation(Vec3f(sineWave, 170.f + sineWave2,-250 + time * ghostSpeed));
					ghost.first.trans->setRotation(Quaternion(Vec3f(0,0,1),osgDegree2Rad(90*sineWave2*0.1)));
					//std::cout << "case 0 : " << ghost.first.ghostId % 4 << '\n';
					if(-250 + time * ghostSpeed > 0 && -250 + time * ghostSpeed < 1){
						root->subChild(ghost.second);
					}
					break;
				// LINKS
				case (1):
					//std::cout << "case 1 und id ->" << ghost.first.ghostId << '\n';
					//std::cout << root->getNChildren() << '\n';
					ghost.first.trans->setTranslation(Vec3f(-250 + time * ghostSpeed,sineWave + 170.f,sineWave2));
					ghost.first.trans->setRotation(Quaternion(Vec3f(0,1,0),osgDegree2Rad(90*sineWave2*0.1)));
					//TODO : remove ghost from scene
					//std::cout << -250 + time * ghostSpeed << '\n';
					if(-250 + time * ghostSpeed > 0 && -250 + time * ghostSpeed < 1)
					{
						root->subChild(ghost.second);
					}
					break;
				// RECHTS
				case (2):
					//std::cout << "case 2: " << ghost.first.ghostId % 4 << '\n';
					ghost.first.trans->setTranslation(Vec3f(250 - time * ghostSpeed,sineWave + 170.f,sineWave2));
					ghost.first.trans->setRotation(Quaternion(Vec3f(0,1,0),osgDegree2Rad(-90*sineWave2*0.1)));
					if(-250 + time * ghostSpeed > 0 && -250 + time * ghostSpeed < 1)
					{
					root->subChild(ghost.second);
			
					}
					break;
				// OBEN
				case (3):
					//std::cout << "case 3: " << ghost.first.ghostId % 4 << '\n';
					ghost.first.trans->setTranslation(Vec3f(sineWave2,450 - time * ghostSpeed,-70 + sineWave));
					ghost.first.trans->setRotation(Quaternion(Vec3f(1,0,0),osgDegree2Rad(60)));
					if(-380 + time * ghostSpeed > 0 && -380 + time * ghostSpeed < 1) // 450-170
					{
						root->subChild(ghost.second);
					}
					break;
			};

			//ghost.trans->setTranslation(Vec3f(ghost.id * 5 * sineWave,time * sineWave, time));
		}
		//------------------------------------------------------------------------------


		//std::cout << "mod time: " << std::fmod(timeSinceStart, 10.00f) << '\n';
		commitChanges();
		mgr->redraw();
		// the changelist should be cleared - else things could be copied multiple times
		OSG::Thread::getCurrentChangeList()->clear();
	});
	glutReshapeFunc([](int w, int h)
	{
		mgr->resize(w, h);
		glutPostRedisplay();
	});
	glutKeyboardFunc(keyboard);
	glutIdleFunc([]() //hier zeug in echtzeit 
	{
		glutPostRedisplay();
	});
}

//------------------------------------------------------------------------------
// Main Function
//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
//#if WIN32
	OSG::preloadSharedObject("OSGFileIO");
	OSG::preloadSharedObject("OSGImageFileIO");
//#endif
	try
	{
		bool cfgIsSet = false;
		NodeRefPtr scene = nullptr;

		// ChangeList needs to be set for OpenSG 1.4
		ChangeList::setReadWriteDefault();
		osgInit(argc,argv);

		// evaluate intial params
		for(int a=1 ; a<argc ; ++a)
		{
			if( argv[a][0] == '-' )
			{
				if ( strcmp(argv[a],"-f") == 0 ) 
				{
					char* cfgFile = argv[a][2] ? &argv[a][2] : &argv[++a][0];
					if (!cfg.loadFile(cfgFile)) 
					{
						std::cout << "ERROR: could not load config file '" << cfgFile << "'\n";
						return EXIT_FAILURE;
					}
					cfgIsSet = true;
				}
			} else {
				std::cout << "Loading scene file '" << argv[a] << "'\n";
				scene = SceneFileHandler::the()->read(argv[a], NULL);
			}
		}

		// load the CAVE setup config file if it was not loaded already:
		if (!cfgIsSet) 
		{
			const char* const default_config_filename = "config/mono.csm";
			if (!cfg.loadFile(default_config_filename)) 
			{
				std::cout << "ERROR: could not load default config file '" << default_config_filename << "'\n";
				return EXIT_FAILURE;
			}
		}

		cfg.printConfig();

		setupGLUT(&argc, argv);

		InitTracker(cfg);

		MultiDisplayWindowRefPtr mwin = createAppWindow(cfg, cfg.getBroadcastaddress());

		if (!scene) 
			scene = buildScene();
		commitChanges();

		mgr = new OSGCSM::CAVESceneManager(&cfg);
		mgr->setWindow(mwin );
		mgr->setRoot(scene);
		mgr->showAll();

		//Background for each wall
		ImageRecPtr backimage = Image::create();
		backimage->read("models/mansion1.jpg");
		TextureObjChunkRecPtr bkgTex = TextureObjChunk::create();
		bkgTex->setImage(backimage);
		bkgTex->setScale(false);
		TextureBackgroundRecPtr imBkg = TextureBackground::create();
		imBkg->setTexture(bkgTex);
		imBkg->setColor(Color4f(1.0,1.0,1.0,0.0f));
		for (int i = 0; i < cfg.getNumActiveWalls(); i++)
		{
			mgr->setBackground(i, imBkg);
		}

		mgr->getWindow()->init();
		mgr->turnWandOff();
	}
	catch(const std::exception& e)
	{
		std::cout << "ERROR: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	glutMainLoop();
}
