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

## Reexportar os modelos

Na pasta `modelos`, há um script Python para cada `.blend`: `exportar_helicoptero.py`, `exportar_carousel.py`, `exportar_kamikaze.py`, `exportar_wave_swinger.py` e `exportar_cha_viking.py`. Execute com o Blender 5.2.1, a partir dessa pasta. Por exemplo:

```sh
blender -b Carousel.blend --python exportar_carousel.py
```

Troque os nomes do `.blend` e do script conforme o modelo. Os scripts de Carousel, Kamikaze, Wave Swinger e Chá Viking gravam OBJ, MTL e texturas em `projetos/<modelo>/`. O Helicóptero grava primeiro em `projetos/helicopter/reexportacao/`; após conferir o resultado, execute novamente com `-- --substituir` para gravar no diretório do programa. O arquivo Blender permanece montado e não é salvo pelo script. O Chá Viking usa também `pivos_cha_viking.json` e gera as texturas das canecas por bake.

Os quatro modelos com pivôs reproduziram os OBJ e as texturas atuais em uma execução de teste com Blender 5.2.1. Os MTL também conservaram os materiais; as diferenças foram apenas o nome do arquivo Blender no comentário de Carousel e Kamikaze e a quebra de linha de Wave Swinger. O Chá Viking reproduziu também o MTL byte a byte.

O Helicóptero foi exportado com peças locais, triangulação, eixos `-Z/Y` e texturas de cor compatíveis com o leitor VART. Seu OBJ atual tem 70.032 triângulos e veio de uma conversão antiga; a exportação direta do `.blend` preparado gera 70.060. Por isso, antes de substituir o OBJ do projeto, confira visualmente a montagem e a rotação das hélices no aplicativo. A diferença histórica está detalhada em `../APRENDIZADOS_EXPORTACAO.md`.
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
