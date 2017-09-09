
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
/*----------1.2----------*/
/*----------2.2----------*/
/*----------3.2----------*/
/*----------4.2----------*/
/*----------6.2----------*/
/*----------7.3----------*/

#ifdef _MSC_VER
# pragma warning(pop)
#endif

OSG_USING_NAMESPACE // activate the OpenSG namespace

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
OSG::SimpleSceneManagerRefPtr mgr; // the SimpleSceneManager to manage applications
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

/*----------1.1----------*/

/*----------2.1----------*/

/*----------3.1----------*/

/*----------4.1----------*/


/*----------5.1----------*/


/*----------7.1----------*/

	return NodeTransitPtr(root);
}

/*----------9.1----------*/

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

	/*----------9.2----------*/

		mgr = OSG::SimpleSceneManager::create();
		mgr->setWindow(gwin);			// tell the manager what to manage
		mgr->setRoot(root);				// attach the scenegraph
	/*----------7.2----------*/
		mgr->showAll();					// show the whole scene

	/*----------6.1----------*/

	}
	glutMainLoop(); // GLUT main loop
}


void display() {
/*----------8.1----------*/

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
