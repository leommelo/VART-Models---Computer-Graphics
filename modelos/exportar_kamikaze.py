"""Exporta malhas locais, com cada pivo na origem do seu objeto OBJ.

Uso: blender -b Kamikaze.blend --python exportar_kamikaze.py
O .blend continua montado; as posicoes sao zeradas somente para exportar.
"""

from pathlib import Path

import bpy


DEST = Path(__file__).resolve().parent.parent / 'projetos' / 'kamikaze'
EXPECTED = {'Base', 'Rod', 'Plataform', 'Chair'}
PIVOTS = {
    'Base': (0.0, 0.0, 0.0),
    'Rod': (0.0, 0.0, 7.793174743652344),
    'Plataform': (0.0, 0.0, 0.6151113510131836),
    'Chair': (0.0, 0.0, 0.6151113510131836),
}

meshes = {obj.name for obj in bpy.data.objects if obj.type == 'MESH'}
if meshes != EXPECTED:
    raise RuntimeError(f'Pecas inesperadas: {meshes}; esperado: {EXPECTED}')

for name, pivot in PIVOTS.items():
    obj = bpy.data.objects[name]
    if any(abs(obj.matrix_world.translation[i] - pivot[i]) > 1e-5 for i in range(3)):
        raise RuntimeError(f'Pivo incorreto em {name}: {tuple(obj.matrix_world.translation)}')

# A pintura XINAS esta embutida no .blend. O OBJ precisa de um arquivo
# separado para que o VART consiga carregar a imagem pelo MTL.
logo = bpy.data.images.get('BaseColorWhite')
if logo is None or logo.packed_file is None:
    raise RuntimeError('Pintura XINAS nao encontrada no .blend.')
texture_dir = DEST / 'textures'
texture_dir.mkdir(exist_ok=True)
logo_path = texture_dir / 'Kamikaze_XINAS.png'
logo_path.write_bytes(bytes(logo.packed_file.data))
export_logo = bpy.data.images.load(str(logo_path), check_existing=False)
for material in bpy.data.materials:
    if material.node_tree:
        for node in material.node_tree.nodes:
            if node.type == 'TEX_IMAGE' and node.image == logo:
                node.image = export_logo

# O leitor de MTL do VART nao interpreta mapas de normal/roughness. Removemos
# somente essas ligacoes na cena aberta em memoria; texturas de Base Color
# permanecem conectadas e devem acompanhar o OBJ exportado.
for material in bpy.data.materials:
    if material.node_tree:
        for link in list(material.node_tree.links):
            if link.from_node.type == 'TEX_IMAGE' and link.to_socket.name != 'Base Color':
                material.node_tree.links.remove(link)

# Remove o parentesco temporariamente e zera a posicao no mundo. Chair e
# filho de Plataform no .blend; zerar apenas location nao zeraria seu pivo.
original_matrices = {name: bpy.data.objects[name].matrix_world.copy() for name in EXPECTED}
original_parents = {name: bpy.data.objects[name].parent for name in EXPECTED}
for name in EXPECTED:
    obj = bpy.data.objects[name]
    obj.parent = None
    local_matrix = original_matrices[name].copy()
    local_matrix.translation = (0, 0, 0)
    obj.matrix_world = local_matrix
bpy.context.view_layer.update()

bpy.ops.wm.obj_export(
    filepath=str(DEST / 'Kamikaze.obj'),
    export_selected_objects=False,
    apply_modifiers=False,
    export_triangulated_mesh=True,
    export_materials=True,
    path_mode='RELATIVE',
    forward_axis='Y',
    up_axis='Z',
)

for name in EXPECTED:
    obj = bpy.data.objects[name]
    obj.parent = original_parents[name]
    obj.matrix_world = original_matrices[name]
bpy.context.view_layer.update()

# O exportador escreve Ka branco e Ks 0.5 para todos os materiais. Com a luz
# ambiente do VART isso lava as cores; usamos ambiente proporcional ao Kd.
mtl = DEST / 'Kamikaze.mtl'
lines = []
textured_material = False
for line in mtl.read_text(encoding='utf-8').splitlines():
    if line.startswith('newmtl '):
        textured_material = line == 'newmtl Metal_Branco'
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
        line = 'map_Kd textures/Kamikaze_XINAS.png'
    lines.append(line)
    if line.startswith('Kd '):
        color = [float(value) * 0.3 for value in line.split()[1:4]]
        lines.append('Ka ' + ' '.join(f'{value:.6f}' for value in color))
if 'map_Kd textures/Kamikaze_XINAS.png' not in lines:
    raise RuntimeError('O MTL nao recebeu a pintura XINAS.')
# O leitor VART reutiliza a textura ao iniciar o material seguinte.
# Deixar o material texturizado por ultimo evita aplicar o logo aos demais.
start = lines.index('newmtl Metal_Branco')
end = next((i for i in range(start + 1, len(lines))
            if lines[i].startswith('newmtl ')), len(lines))
painted = lines[start:end]
del lines[start:end]
lines.extend(painted)
mtl.write_text('\n'.join(lines) + '\n', encoding='utf-8')
print('OBJ exportado:', DEST / 'Kamikaze.obj')
