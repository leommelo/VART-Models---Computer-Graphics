// Carrossel: a mesma malha de cavalo e poste aparece em quatro ramos da cena.
// As malhas do OBJ estao relativas aos pivos definidos no Blender.

#include <vart/scene.h>
#include <vart/light.h>
#include <vart/meshobject.h>
#include <vart/transform.h>
#include <vart/contrib/viewerglutogl.h>

#include <cmath>
#include <iostream>
#include <list>

using namespace std;

const int NUM_CAVALOS = 4;
const double PI = 3.141592653589793;
const double PASSO_ROTACAO = 0.01; // radianos por chamada de OnIdle
const double PASSO_SUBIDA = 0.04;  // radianos por chamada de OnIdle
const double AMPLITUDE_SUBIDA = 1.6;
const double PIVO_CAVALO_X = 0.11513654887676239;
const double PIVO_CAVALO_Y = 0.9080419540405273;
const double PIVO_CAVALO_Z = -8.379340171813965;

class MyIHClass : public VART::ViewerGlutOGL::IdleHandler
{
    public:
        MyIHClass() : carouselRotPtr(nullptr), horseTranslPtr(nullptr),
                      rotacaoRad(0), subidaRad(0) {}

        virtual void OnIdle() {
            rotacaoRad += PASSO_ROTACAO;
            subidaRad += PASSO_SUBIDA;
            carouselRotPtr->MakeYRotation(rotacaoRad);

            for (int i = 0; i < NUM_CAVALOS; ++i) {
                // A fase diferente faz os cavalos subirem em momentos diferentes.
                double fase = i * (2 * PI / NUM_CAVALOS);
                double altura = AMPLITUDE_SUBIDA * (1 + sin(subidaRad + fase));
                horseTranslPtr[i].MakeTranslation(0, altura, 0);
            }

            viewerPtr->PostRedisplay();
        }

        VART::Transform* carouselRotPtr;
        VART::Transform* horseTranslPtr;
    private:
        double rotacaoRad;
        double subidaRad;
};

int main(int argc, char* argv[])
{
    VART::ViewerGlutOGL::Init(&argc, argv);
    static VART::Scene scene;
    static VART::ViewerGlutOGL viewer(900, 700);
    VART::Camera camera(VART::Point4D(36, 25, 36),
                        VART::Point4D(0, 10, 0), VART::Point4D::Y());

    // Le as tres pecas exportadas com os pivos na origem de cada malha.
    list<VART::MeshObject*> objects;
    try {
        if (!VART::MeshObject::ReadFromOBJ("Carousel.obj", &objects)) {
            cerr << "Nao foi possivel carregar Carousel.obj.\n";
            return 1;
        }
    }
    catch (...) {
        cerr << "Falha ao carregar a exportacao. Confira o erro acima.\n";
        return 1;
    }

    VART::MeshObject* base = nullptr;
    VART::MeshObject* horse = nullptr;
    VART::MeshObject* pole = nullptr;
    for (auto* object : objects) {
        string name = object->GetDescription();
        if (name == "Base")
            base = object;
        else if (name == "Horse")
            horse = object;
        else if (name == "HorsePole")
            pole = object;
    }

    if (!base || !horse || !pole) {
        cerr << "O OBJ precisa conter Base, Horse e HorsePole.\n";
        return 1;
    }

    // As duas malhas compartilhadas nao podem ser apagadas por cada pai.
    horse->autoDelete = false;
    pole->autoDelete = false;

    // Mantem a base parada e gira os quatro conjuntos pelo centro.
    VART::Transform carouselRotation;
    carouselRotation.MakeIdentity();
    scene.AddObject(base);
    scene.AddObject(&carouselRotation);

    VART::Transform horsePositions[NUM_CAVALOS];
    VART::Transform horseSupports[NUM_CAVALOS];
    VART::Transform horseMovements[NUM_CAVALOS];
    for (int i = 0; i < NUM_CAVALOS; ++i) {
        // Posiciona quatro copias em intervalos de 90 graus.
        double angulo = i * (2 * PI / NUM_CAVALOS);
        horsePositions[i].MakeYRotation(angulo);
        carouselRotation.AddChild(horsePositions[i]);

        // Leva o pivo do cavalo e do poste ate a haste de apoio.
        horseSupports[i].MakeTranslation(PIVO_CAVALO_X, PIVO_CAVALO_Y,
                                          PIVO_CAVALO_Z);
        horsePositions[i].AddChild(horseSupports[i]);
        horseSupports[i].AddChild(*pole);

        // Somente o cavalo desliza verticalmente ao longo do poste.
        double altura = AMPLITUDE_SUBIDA * (1 + sin(angulo));
        horseMovements[i].MakeTranslation(0, altura, 0);
        horseSupports[i].AddChild(horseMovements[i]);
        horseMovements[i].AddChild(*horse);
    }

    VART::Light light("Luz do carrossel", 0.8f, 0.1f,
                      VART::Color::WHITE(), VART::Point4D(25, 30, 20));
    scene.AddLight(light);
    scene.AddCamera(&camera);

    MyIHClass idleHandler;
    idleHandler.carouselRotPtr = &carouselRotation;
    idleHandler.horseTranslPtr = horseMovements;
    viewer.SetTitle("V-ART Carousel");
    viewer.SetScene(scene);
    scene.MakeCameraViewAll();
    // A camera automatica olha de cima; baixamos para mostrar os cavalos.
    camera.SetLocation(VART::Point4D(36, 17, 36));
    camera.SetTarget(VART::Point4D(0, 10, 0));
    viewer.SetIdleHandler(&idleHandler);

    cout << "Carrossel com " << NUM_CAVALOS << " cavalos e postes.\n";
    scene.DrawLightsOGL();
    VART::ViewerGlutOGL::MainLoop();
    return 0;
}
