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

Cada programa contém `main.cpp`, `Makefile`, OBJ, MTL e somente as imagens referenciadas pelos materiais. O V-ART é compartilhado por meio de `projetos/vart`, e os Makefiles utilizam essa dependência comum.

Os arquivos Blender contêm os modelos montados e as imagens incorporadas. Os exportadores geram as malhas OBJ em relação aos pivôs definidos nesses arquivos, e os grafos dos programas estabelecem as posições de montagem. Alterações nos pivôs exigem conferir as transformações de posicionamento em C++. A compilação utiliza os recursos já exportados e não executa o Blender.

## Exportar os modelos

Na pasta `modelos`, há um script Python para cada `.blend`: `exportar_helicoptero.py`, `exportar_carousel.py`, `exportar_kamikaze.py`, `exportar_wave_swinger.py` e `exportar_cha_viking.py`. Execute com o Blender 5.2.1, a partir dessa pasta. Por exemplo:

```sh
blender -b Carousel.blend --python exportar_carousel.py
```

Troque os nomes do `.blend` e do script conforme o modelo. Cada script grava OBJ, MTL e texturas diretamente no diretório da aplicação correspondente em `projetos/`. O arquivo Blender permanece montado e não é salvo pelo script. O Chá Viking gera as texturas das canecas por bake.

Os scripts registram a triangulação, as convenções de eixos, a exportação das peças em coordenadas locais e a preparação dos materiais e imagens para o V-ART. Os caminhos de saída são definidos a partir da localização do script, preservando a organização do repositório.

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
