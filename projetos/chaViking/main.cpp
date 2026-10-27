// Cha Viking: pendulo no apoio, giro da haste e giro de cada caneca.
// O OBJ preserva Z para cima e traz cada malha relativa ao seu pivo.

#include <vart/scene.h>
#include <vart/light.h>
#include <vart/meshobject.h>
#include <vart/transform.h>
#include <vart/contrib/viewerglutogl.h>

#include <cmath>
#include <iostream>
#include <list>
#include <string>

using namespace std;

const int NUM_CANECAS = 2;
const double PI = 3.141592653589793;
const double AMPLITUDE_PENDULO = 45.0 * PI / 180.0;
const double PASSO_PENDULO = 0.006; // fase em radianos por chamada de OnIdle
const double PASSO_HASTE = 0.008;   // radianos por chamada de OnIdle
const double PASSO_CANECA = 0.0125;  // radianos por chamada de OnIdle

const double PIVO_HASTE_X = 0.0007280822610482574;
const double PIVO_HASTE_Z = 15.074895858764648;
const double PIVO_CANECA_X = 0.013481243513524532;
const double PIVO_CANECA_Y = 3.275329351425171;
const double PIVO_CANECA_Z = 2.32802677154541;

class MyIHClass : public VART::ViewerGlutOGL::IdleHandler {
public:
    MyIHClass() : penduloRot(nullptr), hasteRot(nullptr), canecasRot(nullptr),
                  fasePendulo(0), hasteRad(0), canecasRad(0) {}

    void OnIdle() override {
        fasePendulo += PASSO_PENDULO;
        hasteRad += PASSO_HASTE;
        canecasRad += PASSO_CANECA;

        // O seno desacelera o pendulo nas extremidades do balanco.
        penduloRot->MakeXRotation(AMPLITUDE_PENDULO * sin(fasePendulo));
        hasteRot->MakeZRotation(hasteRad);
        for (int i = 0; i < NUM_CANECAS; ++i)
            canecasRot[i].MakeZRotation(canecasRad);

        viewerPtr->PostRedisplay();
    }

    VART::Transform* penduloRot;
    VART::Transform* hasteRot;
    VART::Transform* canecasRot;
private:
    double fasePendulo;
    double hasteRad;
    double canecasRad;
};

int main(int argc, char* argv[]) {
    VART::ViewerGlutOGL::Init(&argc, argv);
    static VART::Scene scene;
    static VART::ViewerGlutOGL viewer(900, 700);
    VART::Camera camera(VART::Point4D(22, -26, 18),
                        VART::Point4D(0, 0, 7), VART::Point4D::Z());

    list<VART::MeshObject*> objects;
    try {
        if (!VART::MeshObject::ReadFromOBJ("ChaViking.obj", &objects)) {
            cerr << "Nao foi possivel carregar ChaViking.obj.\n";
            return 1;
        }
    } catch (...) {
        cerr << "Falha ao carregar o modelo ou suas texturas.\n";
        return 1;
    }

    VART::MeshObject* base = nullptr;
    VART::MeshObject* eixo = nullptr;
    VART::MeshObject* haste = nullptr;
    VART::MeshObject* canecas[NUM_CANECAS] = {};
    VART::MeshObject* rebites[NUM_CANECAS] = {};
    for (auto* object : objects) {
        const string name = object->GetDescription();
        if (name == "Base") base = object;
        else if (name == "Eixo") eixo = object;
        else if (name == "Haste") haste = object;
        else {
            for (int i = 0; i < NUM_CANECAS; ++i) {
                if (name == "Caneca_" + to_string(i)) canecas[i] = object;
                if (name == "Rebites_" + to_string(i)) rebites[i] = object;
            }
        }
    }
    if (!base || !eixo || !haste) {
        cerr << "O OBJ precisa conter Base, Eixo e Haste.\n";
        return 1;
    }
    for (int i = 0; i < NUM_CANECAS; ++i) {
        if (!canecas[i] || !rebites[i]) {
            cerr << "Caneca ou rebites ausentes: " << i << ".\n";
            return 1;
        }
    }

    VART::Transform pivoHaste;
    VART::Transform pendulo;
    VART::Transform giroHaste;
    VART::Transform pivosCanecas[NUM_CANECAS];
    VART::Transform girosCanecas[NUM_CANECAS];

    // Coloca o pivo no eixo horizontal superior.
    pivoHaste.MakeTranslation(PIVO_HASTE_X, 0, PIVO_HASTE_Z);
    pendulo.MakeIdentity();
    giroHaste.MakeIdentity();
    scene.AddObject(base);
    scene.AddObject(&pivoHaste);
    pivoHaste.AddChild(*eixo);
    pivoHaste.AddChild(pendulo);
    pendulo.AddChild(giroHaste);
    giroHaste.AddChild(*haste);

    for (int i = 0; i < NUM_CANECAS; ++i) {
        // Posiciona cada caneca na ponta da travessa inferior.
        const double lado = (i == 0 ? -1.0 : 1.0);
        pivosCanecas[i].MakeTranslation(PIVO_CANECA_X - PIVO_HASTE_X,
                                       lado * PIVO_CANECA_Y,
                                       PIVO_CANECA_Z - PIVO_HASTE_Z);
        girosCanecas[i].MakeIdentity();
        giroHaste.AddChild(pivosCanecas[i]);
        pivosCanecas[i].AddChild(girosCanecas[i]);
        girosCanecas[i].AddChild(*canecas[i]);
        girosCanecas[i].AddChild(*rebites[i]);
    }

    scene.AddLight(VART::Light::BRIGHT_AMBIENT());
    scene.AddCamera(&camera);
    MyIHClass idleHandler;
    idleHandler.penduloRot = &pendulo;
    idleHandler.hasteRot = &giroHaste;
    idleHandler.canecasRot = girosCanecas;
    viewer.SetTitle("V-ART Cha Viking");
    viewer.SetScene(scene);
    viewer.SetIdleHandler(&idleHandler);

    cout << "Cha Viking: pendulo, giro da haste e giro de duas canecas.\n";
    scene.DrawLightsOGL();
    VART::ViewerGlutOGL::MainLoop();
    return 0;
}
