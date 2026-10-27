// main.cpp - from V-ART template application

// Renderiza o helicoptero (Helicoptero.obj) exportado do Blender.
// Cada peça vem no OBJ com o pivô na origem; aqui ela é levada para o lugar
// com uma translação e, no caso das hélices, girada em torno do próprio pivô.

#include <vart/scene.h>
#include <vart/light.h>
#include <vart/meshobject.h>
#include <vart/contrib/viewerglutogl.h>

#include <iostream>

using namespace std;

class MyIHClass : public VART::ViewerGlutOGL::IdleHandler
{
    public:
        MyIHClass() : mainRotorRadianos(0), tailRotorRadianos(0) {}
        virtual ~MyIHClass() {}
        virtual void OnIdle() {
            mainRotorRadianos += 0.16;
            mainRotorRotPtr->MakeYRotation(mainRotorRadianos);

            tailRotorRadianos += 0.20;
            tailRotorRotPtr->MakeXRotation(tailRotorRadianos);

            viewerPtr->PostRedisplay();
        }
    //protected:
        VART::Transform* mainRotorRotPtr;
        VART::Transform* tailRotorRotPtr;
    private:
        float mainRotorRadianos;
        float tailRotorRadianos;
};

// The application itself:
int main(int argc, char* argv[])
{
    VART::ViewerGlutOGL::Init(&argc, argv); // Initialize GLUT
    static VART::Scene scene; // create a scene
    static VART::ViewerGlutOGL viewer; // create a viewer (application window)
    // create a camera (scene observer)
    VART::Camera camera(VART::Point4D(30,20,40),VART::Point4D::ORIGIN(),VART::Point4D::Y());

    //posição de cada peça
    // Location do Blender (x, y, z) convertido para o OBJ: (x, z, -y)
    VART::Transform bodyTransl;
    bodyTransl.MakeTranslation(0, 4.512123, -0.290335);
    VART::Transform mainRotorTransl;
    mainRotorTransl.MakeTranslation(0, 7.885469, 0.791599);
    VART::Transform tailRotorTransl;
    tailRotorTransl.MakeTranslation(-0.082102, 6.983313, -16.876935);
    VART::Transform skidsTransl;
    skidsTransl.MakeTranslation(0, 0.059297, 0.003349);
    VART::Transform missilesTransl;
    missilesTransl.MakeTranslation(0, 0.059297, 0.003349);
    VART::Transform baseTransl;
    baseTransl.MakeTranslation(0, -0.107590, -3.201585);

    //rotação das hélices
    VART::Transform mainRotorRotation;
    mainRotorRotation.MakeIdentity();
    mainRotorTransl.AddChild(mainRotorRotation);
    VART::Transform tailRotorRotation;
    tailRotorRotation.MakeIdentity();
    tailRotorTransl.AddChild(tailRotorRotation);

    scene.AddObject(&bodyTransl);
    scene.AddObject(&mainRotorTransl);
    scene.AddObject(&tailRotorTransl);
    scene.AddObject(&skidsTransl);
    scene.AddObject(&missilesTransl);
    scene.AddObject(&baseTransl);

    //Leitura
    list<VART::MeshObject*> objects;
    VART::MeshObject::ReadFromOBJ("Helicoptero.obj", &objects);
    list<VART::MeshObject*>::iterator iter = objects.begin();
    for (; iter != objects.end(); ++iter) {
        string name = (*iter)->GetDescription();
        if (name == "Helicopter_Body"){
            bodyTransl.AddChild(**iter);
        }else if (name == "Main_Rotor"){
            mainRotorRotation.AddChild(**iter);
        }else if (name == "Tail_Rotor"){
            tailRotorRotation.AddChild(**iter);
        }else if (name == "Landing_Skids"){
            skidsTransl.AddChild(**iter);
        }else if (name == "Missiles"){
            missilesTransl.AddChild(**iter);
        }else if (name == "Cylinder"){
            baseTransl.AddChild(**iter);
        }
        else{
            scene.AddObject(*iter);
        }
    }

    MyIHClass idleHandler;
    idleHandler.mainRotorRotPtr = &mainRotorRotation;
    idleHandler.tailRotorRotPtr = &tailRotorRotation;

    // Add lights and cameras to the scene
    scene.AddLight(VART::Light::BRIGHT_AMBIENT());
    scene.AddCamera(&camera);
    scene.MakeCameraViewAll();

    // Set up the viewer
    viewer.SetTitle("V-ART helicoptero");
    viewer.SetScene(scene); // attach the scene
    viewer.SetIdleHandler(&idleHandler);

    // Run application
    scene.DrawLightsOGL(); // Set OpenGL's lights' state
    VART::ViewerGlutOGL::MainLoop(); // Enter main loop (event loop) and never return
    return 0;
}
