// Kamikaze: Base -> pivo da haste -> rotacao Y -> pivo dos assentos
//           -> rotacao Z -> plataforma e assentos.
// O OBJ preserva os eixos do Blender (Z para cima) e traz cada malha local
// ao seu pivo. As translacoes abaixo montam as pecas na cena.

#include <vart/scene.h>
#include <vart/light.h>
#include <vart/meshobject.h>
#include <vart/transform.h>
#include <vart/contrib/viewerglutogl.h>

#include <iostream>
#include <list>
#include <string>

namespace {

const double PIVO_HASTE_Z = 7.793174743652344;
const double PIVO_ASSENTOS_Z = 0.6151113510131836;
const float PASSO_HASTE = 0.02f;   // radianos por chamada de OnIdle
const float PASSO_ASSENTOS = 0.02f; // radianos por chamada de OnIdle

class MyIHClass : public VART::ViewerGlutOGL::IdleHandler
{
    public:
        MyIHClass() : hasteRad(0), assentosRad(0) {}
        virtual ~MyIHClass() {}

        virtual void OnIdle() {
            hasteRad += PASSO_HASTE;
            hasteRot->MakeYRotation(hasteRad);
            assentosRad += PASSO_ASSENTOS;
            assentosRot->MakeZRotation(assentosRad);
            viewerPtr->PostRedisplay();
        }

        VART::Transform* hasteRot;
        VART::Transform* assentosRot;
    private:
        float hasteRad;
        float assentosRad;
};

} // namespace

int main(int argc, char* argv[]) {
    VART::ViewerGlutOGL::Init(&argc, argv);
    static VART::Scene scene;
    static VART::ViewerGlutOGL viewer(900, 700);
    VART::Camera camera(VART::Point4D(23, -28, 17),
                        VART::Point4D(0, 0, 7), VART::Point4D::Z());

    std::list<VART::MeshObject*> objects;
    try {
        if (!VART::MeshObject::ReadFromOBJ("Kamikaze.obj", &objects)) {
            std::cerr << "Nao foi possivel carregar Kamikaze.obj.\n";
            return 1;
        }
    } catch (...) {
        std::cerr << "Falha ao carregar Kamikaze.obj ou Kamikaze.mtl.\n";
        return 1;
    }

    VART::MeshObject* base = nullptr;
    VART::MeshObject* rod = nullptr;
    VART::MeshObject* platform = nullptr;
    VART::MeshObject* chair = nullptr;
    for (auto* object : objects) {
        const std::string name = object->GetDescription();
        if (name == "Base") base = object;
        else if (name == "Rod") rod = object;
        else if (name == "Plataform") platform = object;
        else if (name == "Chair") chair = object;
    }
    if (!base || !rod || !platform || !chair) {
        std::cerr << "O OBJ precisa conter Base, Rod, Plataform e Chair.\n";
        return 1;
    }

    VART::Transform pivoHaste;
    VART::Transform rotacaoHaste;
    VART::Transform pivoAssentos;
    VART::Transform rotacaoAssentos;

    // Posiciona a origem da rotacao da haste no eixo de apoio.
    pivoHaste.MakeTranslation(0, 0, PIVO_HASTE_Z);
    rotacaoHaste.MakeIdentity();

    // Posiciona o pivo dos assentos na ponta da haste.
    pivoAssentos.MakeTranslation(0, 0, PIVO_ASSENTOS_Z - PIVO_HASTE_Z);
    rotacaoAssentos.MakeIdentity();

    // Adiciona a base fixa e a haste ao grafo da cena.
    scene.AddObject(base);
    scene.AddObject(&pivoHaste);
    pivoHaste.AddChild(rotacaoHaste);
    rotacaoHaste.AddChild(*rod);

    // Adiciona a rotacao dos assentos como filha da rotacao da haste.
    rotacaoHaste.AddChild(pivoAssentos);
    pivoAssentos.AddChild(rotacaoAssentos);
    rotacaoAssentos.AddChild(*platform);
    rotacaoAssentos.AddChild(*chair);

    scene.AddLight(VART::Light::BRIGHT_AMBIENT());
    scene.AddCamera(&camera);
    MyIHClass idleHandler;
    idleHandler.hasteRot = &rotacaoHaste;
    idleHandler.assentosRot = &rotacaoAssentos;
    viewer.SetTitle("V-ART Kamikaze");
    viewer.SetScene(scene);
    viewer.SetIdleHandler(&idleHandler);
    scene.DrawLightsOGL();
    VART::ViewerGlutOGL::MainLoop();
    return 0;
}
