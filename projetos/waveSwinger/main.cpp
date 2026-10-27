// Wave Swinger: as setas ajustam cinco niveis de giro e inclinacao lateral.

#include <vart/scene.h>
#include <vart/light.h>
#include <vart/meshobject.h>
#include <vart/transform.h>
#include <vart/contrib/viewerglutogl.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <string>

using namespace std;

namespace {

const int NUM_CADEIRAS = 8;
const double PI = 3.141592653589793;
const int NIVEL_MAXIMO = 5;
const float PASSO_GIRO_POR_NIVEL = -0.02f; // radianos por chamada de OnIdle
const float PASSO_INCLINACAO = 0.003f; // radianos por chamada de OnIdle
const float INCLINACAO_POR_NIVEL = 5.0 * PI / 180.0;

// Ponto superior das correntes da Seat_0 no Blender; as outras sete cadeiras
// sao copias espacadas de 45 graus em torno do eixo Z.
const double SUPORTE_X = 3.547271;
const double SUPORTE_Y = 1.348529;
const double SUPORTE_Z = 7.280096;
const double PIVO_PLATAFORMA_Z = 4.72332763671875;

class MyIHClass : public VART::ViewerGlutOGL::IdleHandler {
public:
    MyIHClass() : nivel(0), giroRad(0), inclinacaoRad(0) {}

    void MudarNivel(int passo) {
        const int novoNivel = max(0, min(NIVEL_MAXIMO, nivel + passo));
        if (novoNivel == nivel) return;
        nivel = novoNivel;
        cout << "Nivel " << nivel << "/" << NIVEL_MAXIMO
                  << " - inclinacao: " << nivel * 5 << " graus.\n";
    }

    void OnIdle() override {
        if (nivel == 0 && inclinacaoRad == 0) return;
        giroRad += nivel * PASSO_GIRO_POR_NIVEL;
        giroRot->MakeZRotation(giroRad);

        // Aproxima as cadeiras da inclinacao escolhida, tanto na subida quanto na descida.
        const float inclinacaoAlvo = nivel * INCLINACAO_POR_NIVEL;
        if (inclinacaoRad < inclinacaoAlvo)
            inclinacaoRad = min(inclinacaoRad + PASSO_INCLINACAO, inclinacaoAlvo);
        else if (inclinacaoRad > inclinacaoAlvo)
            inclinacaoRad = max(inclinacaoRad - PASSO_INCLINACAO, inclinacaoAlvo);
        for (int i = 0; i < NUM_CADEIRAS; ++i) {
            // Inclina cada cadeira para fora ao redor do suporte superior.
            inclinacoes[i].MakeRotation(eixosLaterais[i], inclinacaoRad);
        }
        viewerPtr->PostRedisplay();
    }

    VART::Transform* giroRot;
    VART::Transform* inclinacoes;
    VART::Point4D* eixosLaterais;
private:
    int nivel;
    float giroRad;
    float inclinacaoRad;
};

class KeyboardHandler : public VART::ViewerGlutOGL::KbHandler {
public:
    explicit KeyboardHandler(MyIHClass& idleHandler)
        : idleHandler_(&idleHandler), cimaPressionada(false), baixoPressionada(false) {}
    void OnKeyDown(int key) override {
        // Cada tecla muda um nivel ate ser solta novamente.
        if (key == VART::KEY_UP && !cimaPressionada) {
            cimaPressionada = true;
            idleHandler_->MudarNivel(1);
        }
        if (key == VART::KEY_DOWN && !baixoPressionada) {
            baixoPressionada = true;
            idleHandler_->MudarNivel(-1);
        }
    }
    void OnKeyUp(int key) override {
        if (key == VART::KEY_UP) cimaPressionada = false;
        if (key == VART::KEY_DOWN) baixoPressionada = false;
    }
private:
    MyIHClass* idleHandler_;
    bool cimaPressionada;
    bool baixoPressionada;
};

} // namespace

int main(int argc, char* argv[]) {
    VART::ViewerGlutOGL::Init(&argc, argv);
    static VART::Scene scene;
    static VART::ViewerGlutOGL viewer(900, 700);
    VART::Camera camera(VART::Point4D(19, -24, 14),
                        VART::Point4D(0, 0, 4), VART::Point4D::Z());

    list<VART::MeshObject*> objects;
    try {
        if (!VART::MeshObject::ReadFromOBJ("WaveSwinger.obj", &objects)) {
            cerr << "Nao foi possivel carregar WaveSwinger.obj.\n";
            return 1;
        }
    } catch (...) {
        cerr << "Falha ao carregar WaveSwinger.obj ou WaveSwinger.mtl.\n";
        return 1;
    }

    VART::MeshObject* base = NULL;
    VART::MeshObject* grade = NULL;
    VART::MeshObject* plataforma = NULL;
    VART::MeshObject* cadeiras[NUM_CADEIRAS] = {};
    for (auto* object : objects) {
        const string name = object->GetDescription();
        if (name == "Base") base = object;
        else if (name == "Grid") grade = object;
        else if (name == "Plataform") plataforma = object;
        else for (int i = 0; i < NUM_CADEIRAS; ++i)
            if (name == "Seat_" + to_string(i)) cadeiras[i] = object;
    }
    bool todasCadeiras = true;
    for (int i = 0; i < NUM_CADEIRAS; ++i) {
        if (!cadeiras[i]) todasCadeiras = false;
    }
    if (!base || !grade || !plataforma || !todasCadeiras) {
        cerr << "O OBJ precisa conter Base, Grid, Plataform e Seat_0..Seat_7.\n";
        return 1;
    }

    // Posiciona o pivo da plataforma no centro do brinquedo.
    VART::Transform pivoPlataforma;
    pivoPlataforma.MakeTranslation(0, 0, PIVO_PLATAFORMA_Z);
    VART::Transform giro;
    giro.MakeIdentity();
    scene.AddObject(base);
    scene.AddObject(grade);
    scene.AddObject(&pivoPlataforma);
    pivoPlataforma.AddChild(giro);
    giro.AddChild(*plataforma);

    VART::Transform suportes[NUM_CADEIRAS];
    VART::Transform inclinacoes[NUM_CADEIRAS];
    VART::Point4D eixosLaterais[NUM_CADEIRAS];
    for (int i = 0; i < NUM_CADEIRAS; ++i) {
        const double angulo = i * 2.0 * PI / NUM_CADEIRAS;
        const double x = SUPORTE_X * cos(angulo) - SUPORTE_Y * sin(angulo);
        const double y = SUPORTE_X * sin(angulo) + SUPORTE_Y * cos(angulo);
        // O eixo tangente faz a cadeira abrir na direcao oposta ao centro.
        const double raio = sqrt(x * x + y * y);
        eixosLaterais[i] = VART::Point4D(y / raio, -x / raio, 0, 0);
        // Posiciona o pivo de cada cadeira em relacao a plataforma.
        suportes[i].MakeTranslation(x, y, SUPORTE_Z - PIVO_PLATAFORMA_Z);
        inclinacoes[i].MakeIdentity();
        giro.AddChild(suportes[i]);
        suportes[i].AddChild(inclinacoes[i]);
        inclinacoes[i].AddChild(*cadeiras[i]);
    }

    scene.AddLight(VART::Light::BRIGHT_AMBIENT());
    scene.AddCamera(&camera);
    MyIHClass idleHandler;
    idleHandler.giroRot = &giro;
    idleHandler.inclinacoes = inclinacoes;
    idleHandler.eixosLaterais = eixosLaterais;
    KeyboardHandler keyboard(idleHandler);
    viewer.SetTitle("V-ART Wave Swinger - setas ajustam a velocidade");
    viewer.SetScene(scene);
    viewer.SetKbHandler(&keyboard);
    viewer.SetIdleHandler(&idleHandler);
    cout << "Seta para cima aumenta o nivel; seta para baixo diminui (0 a 5).\n";
    scene.DrawLightsOGL();
    VART::ViewerGlutOGL::MainLoop();
    return 0;
}
