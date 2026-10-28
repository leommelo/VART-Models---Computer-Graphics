"""Exporta o carrossel com as origens das malhas no ponto (0, 0, 0).

Uso: blender -b Carousel.blend --python exportar_carousel.py
O arquivo Blender permanece montado: as pecas sao zeradas so em memoria.
"""

from pathlib import Path
import shutil

import bpy


OUTPUT = Path(__file__).resolve().parent.parent / 'projetos' / 'carousel' / 'Carousel.obj'
PIVOTS = {'Base', 'Horse', 'HorsePole'}
objects = {obj.name: obj for obj in bpy.data.objects if obj.type == 'MESH'}
if set(objects) != PIVOTS:
    raise RuntimeError(f'Pecas inesperadas: {set(objects)}')
if (objects['Horse'].matrix_world.translation -
        objects['HorsePole'].matrix_world.translation).length > 1e-5:
    raise RuntimeError('O cavalo e o poste precisam compartilhar o pivo.')
if objects['Base'].matrix_world.translation.length > 1e-5:
    raise RuntimeError('O pivo da base precisa estar no centro do carrossel.')

# Retira o mapa de normal nao suportado pelo leitor MTL do VART.
# A textura de cor do piso continua ligada.
for material in bpy.data.materials:
    if material.node_tree:
        for link in list(material.node_tree.links):
            if link.from_node.type == 'TEX_IMAGE' and link.to_socket.name != 'Base Color':
                material.node_tree.links.remove(link)

# Exporta cada malha relativa a seu pivo, sem salvar essas mudancas no .blend.
original_matrices = {name: obj.matrix_world.copy() for name, obj in objects.items()}
original_parents = {name: obj.parent for name, obj in objects.items()}
for name, obj in objects.items():
    obj.parent = None
    matrix = original_matrices[name].copy()
    matrix.translation = (0, 0, 0)
    obj.matrix_world = matrix
bpy.context.view_layer.update()

bpy.ops.object.select_all(action='DESELECT')
for obj in objects.values():
    obj.select_set(True)
bpy.ops.wm.obj_export(
    filepath=str(OUTPUT),
    export_selected_objects=True,
    apply_modifiers=False,
    export_triangulated_mesh=True,
    export_materials=True,
    path_mode='RELATIVE',
    forward_axis='NEGATIVE_Z',
    up_axis='Y',
)

for name, obj in objects.items():
    obj.parent = original_parents[name]
    obj.matrix_world = original_matrices[name]
bpy.context.view_layer.update()

# Ajusta a luz ambiente sem alterar materiais salvos no Blender.
mtl = OUTPUT.with_suffix('.mtl')
lines = []
textured_material = False
for line in mtl.read_text(encoding='utf-8').splitlines():
    if line.startswith('newmtl '):
        textured_material = line == 'newmtl Carousel_|_Piso_turquesa'
        lines.append(line)
        if textured_material:
            lines.extend(('Kd 1.000000 1.000000 1.000000',
                          'Ka 0.300000 0.300000 0.300000'))
        continue
    if line.startswith(('Ka ', 'map_Bump ', 'bump ')):
        continue
    if textured_material and line.startswith('Kd '):
        continue
    if line.startswith('Ks '):
        line = 'Ks 0.150000 0.150000 0.150000'
    if line.startswith('map_Kd '):
        # O VART aceita somente o nome do arquivo apos map_Kd.
        image = next((image for image in bpy.data.images
                      if image.name == 'Carousel | Piso XINAS'), None)
        if image is None:
            raise RuntimeError('Textura de cor do piso nao encontrada.')
        texture_path = OUTPUT.parent / 'textures' / 'Carousel_Piso_XINAS.png'
        texture_path.parent.mkdir(exist_ok=True)
        if image.packed_file:
            texture_path.write_bytes(bytes(image.packed_file.data))
        else:
            shutil.copy2(bpy.path.abspath(image.filepath), texture_path)
        line = 'map_Kd textures/Carousel_Piso_XINAS.png'
    lines.append(line)
    if line.startswith('Kd '):
        ambient = [float(value) * 0.3 for value in line.split()[1:4]]
        lines.append('Ka ' + ' '.join(f'{value:.6f}' for value in ambient))
if 'map_Kd textures/Carousel_Piso_XINAS.png' not in lines:
    raise RuntimeError('O MTL nao recebeu a pintura XINAS do piso.')
# O leitor VART reutiliza a textura ao iniciar o material seguinte.
# Deixar o material texturizado por ultimo evita pintar outras partes.
start = lines.index('newmtl Carousel_|_Piso_turquesa')
end = next((i for i in range(start + 1, len(lines))
            if lines[i].startswith('newmtl ')), len(lines))
painted = lines[start:end]
del lines[start:end]
lines.extend(painted)
mtl.write_text('\n'.join(lines) + '\n', encoding='utf-8')
print('Exportado:', OUTPUT)
