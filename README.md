# Modelos didáticos de Computação Gráfica com V-ART

Versão 1.0 — Leonardo Marques de Melo.

## Organização

```text
TCC/
├── modelos/
│   ├── Helicoptero.blend
│   ├── Carousel.blend
│   ├── Kamikaze.blend
│   ├── WaveSwinger.blend
│   └── ChaViking.blend
└── projetos/
    ├── helicopter/
    ├── carousel/
    ├── kamikaze/
    ├── waveSwinger/
    ├── chaViking/
    └── vart/
```

Cada programa contém `main.cpp`, `Makefile`, OBJ, MTL e somente as imagens referenciadas pelos materiais. O V-ART é compartilhado por meio de `projetos/vart`, sem links para as pastas originais e sem alterações em seu código. Os fontes e recursos dos programas foram copiados dos projetos em uso; os Makefiles foram adaptados à nova organização.

Os modelos Blender são cópias das versões preparadas para os projetos. As imagens disponíveis foram incorporadas às cópias para facilitar sua abertura em outro computador. Os modelos permanecem montados; as malhas OBJ dos modelos com pivôs locais são posicionadas pelos grafos dos programas. Os arquivos Blender e OBJ não são regenerados pela compilação. No helicóptero, preservar a convenção de exportação e o posicionamento usados pelo código ao reexportar.

## Compilar no Ubuntu/WSL

Instalar as ferramentas e bibliotecas, caso ainda não estejam disponíveis:

```sh
sudo apt install build-essential libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev libdevil-dev
```

Entrar na pasta do programa e executar:

```sh
cd projetos/chaViking
make
./chaViking
```

Programas: `helicopter/helicoptero`, `carousel/carousel`, `kamikaze/kamikaze`, `waveSwinger/waveSwinger` e `chaViking/chaViking`. Executar cada programa dentro de sua própria pasta, pois os recursos usam caminhos relativos. A janela requer uma sessão gráfica, como o WSLg.

O Wave Swinger inicia parado: as setas para cima e para baixo alteram o nível de movimento. Os demais programas iniciam suas animações automaticamente.

## Licenciamento

Consultar `LICENSE` e `LICENCIAMENTO.md`. A proposta MIT abrange as contribuições originais de código. O V-ART, os exemplos preexistentes, modelos e texturas conservam as condições dos respectivos titulares.
