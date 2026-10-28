"""Exporta cada malha do Wave Swinger com seu pivo na origem local.

Uso: blender -b WaveSwinger.blend --python exportar_wave_swinger.py
O .blend permanece montado; as posicoes sao zeradas apenas em memoria.
"""

from pathlib import Path
import math

import bpy


OUTPUT = Path(__file__).resolve().parent.parent / 'projetos' / 'waveSwinger' / 'WaveSwinger.obj'
SEATS = 8
STEP = 2 * math.pi / SEATS
FIRST_HINGE = (3.547271, 1.348529, 7.280096)
PIVOTS = {
    'Base': (0.0, 0.0, 0.0),
    'Grid': (0.0, 0.0, 0.0),
    'Plataform': (0.0, 0.0, 4.72332763671875),
}
for i in range(SEATS):
    angle = i * STEP
    PIVOTS[f'Seat_{i}'] = (
        FIRST_HINGE[0] * math.cos(angle) - FIRST_HINGE[1] * math.sin(angle),
        FIRST_HINGE[0] * math.sin(angle) + FIRST_HINGE[1] * math.cos(angle),
        FIRST_HINGE[2],
    )

meshes = {obj.name for obj in bpy.data.objects if obj.type == 'MESH'}
if meshes != set(PIVOTS):
    raise RuntimeError(f'Pecas inesperadas: {meshes}; esperado: {set(PIVOTS)}')
for name, pivot in PIVOTS.items():
    obj = bpy.data.objects[name]
    if any(abs(obj.matrix_world.translation[axis] - pivot[axis]) > 1e-5
           for axis in range(3)):
        raise RuntimeError(f'Pivo incorreto em {name}: {tuple(obj.matrix_world.translation)}')

# A pintura XINAS do topo esta embutida no .blend e nao tem caminho de arquivo.
# Extraimos a imagem para que o exportador possa referi-la no MTL.
logo = bpy.data.images.get('Creme Base Color.001')
if logo is None or logo.packed_file is None:
    raise RuntimeError('Pintura XINAS do topo nao encontrada no .blend.')
texture_dir = OUTPUT.parent / 'textures'
texture_dir.mkdir(exist_ok=True)
logo_path = texture_dir / 'WaveSwinger_XINAS.png'
logo_path.write_bytes(bytes(logo.packed_file.data))
export_logo = bpy.data.images.load(str(logo_path), check_existing=False)
for material in bpy.data.materials:
    if material.node_tree:
        for node in material.node_tree.nodes:
            if node.type == 'TEX_IMAGE' and node.image == logo:
                node.image = export_logo

# Remove apenas as ligacoes de imagem que o leitor de MTL do VART nao usa.
for material in bpy.data.materials:
    if material.node_tree:
        for link in list(material.node_tree.links):
            if link.from_node.type == 'TEX_IMAGE':
                image = link.from_node.image
                if link.to_socket.name != 'Base Color' or image is None or not image.filepath:
                    material.node_tree.links.remove(link)

# Grid e filho de Base no .blend. O parentesco e as posicoes sao removidos
# apenas na cena em memoria para que cada OBJ seja local ao proprio pivo.
original_matrices = {name: bpy.data.objects[name].matrix_world.copy() for name in PIVOTS}
original_parents = {name: bpy.data.objects[name].parent for name in PIVOTS}
for name in PIVOTS:
    obj = bpy.data.objects[name]
    obj.parent = None
    local_matrix = original_matrices[name].copy()
    local_matrix.translation = (0, 0, 0)
    obj.matrix_world = local_matrix
bpy.context.view_layer.update()

bpy.ops.object.select_all(action='DESELECT')
for name in PIVOTS:
    bpy.data.objects[name].select_set(True)
bpy.ops.wm.obj_export(
    filepath=str(OUTPUT),
    export_selected_objects=True,
    apply_modifiers=False,
    export_triangulated_mesh=True,
    export_materials=True,
    path_mode='RELATIVE',
    forward_axis='Y',
    up_axis='Z',
)

for name in PIVOTS:
    obj = bpy.data.objects[name]
    obj.parent = original_parents[name]
    obj.matrix_world = original_matrices[name]
bpy.context.view_layer.update()

# Mantem as cores legiveis com a iluminacao ambiente do VART.
mtl = OUTPUT.with_suffix('.mtl')
lines = []
textured_material = False
for line in mtl.read_text(encoding='utf-8').splitlines():
    if line.startswith('newmtl '):
        textured_material = line == 'newmtl CremeTopo'
        lines.append(line)
        if textured_material:
            lines.extend(('Kd 1.000000 1.000000 1.000000',
                          'Ka 0.300000 0.300000 0.300000'))
        continue
    if line.startswith('Ka '):
        continue
    if line.startswith('Ks '):
        line = 'Ks 0.150000 0.150000 0.150000'
    if textured_material and line.startswith('Kd '):
        continue
    if textured_material and line.startswith('map_Kd '):
        line = 'map_Kd textures/WaveSwinger_XINAS.png'
    lines.append(line)
    if line.startswith('Kd '):
        color = [float(value) * 0.3 for value in line.split()[1:4]]
        lines.append('Ka ' + ' '.join(f'{value:.6f}' for value in color))
if 'map_Kd textures/WaveSwinger_XINAS.png' not in lines:
    raise RuntimeError('O MTL nao recebeu a pintura XINAS do topo.')
# O leitor VART reutiliza a textura ao iniciar o material seguinte.
# Deixar o material texturizado por ultimo evita aplicar o logo aos demais.
start = lines.index('newmtl CremeTopo')
end = next((i for i in range(start + 1, len(lines))
            if lines[i].startswith('newmtl ')), len(lines))
painted = lines[start:end]
del lines[start:end]
lines.extend(painted)
mtl.write_text('\n'.join(lines) + '\n', encoding='utf-8')
print('Exportado:', OUTPUT)
