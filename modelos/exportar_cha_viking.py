"""Exporta o Cha Viking com malhas locais, pintura XINAS e madeira em PNG.

Uso: blender -b ChaViking.blend --python exportar_cha_viking.py
As mudancas de materiais, UVs e posicoes existem apenas durante a exportacao.
"""

from pathlib import Path
import json
import math

import bpy


DEST = Path(__file__).resolve().parent.parent / 'projetos' / 'chaViking'
OUTPUT = DEST / 'ChaViking.obj'
TEXTURES = DEST / 'textures'
TEXTURES.mkdir(exist_ok=True)
PIVOTS = json.loads((Path(__file__).resolve().parent / 'pivos_cha_viking.json').read_text(encoding='utf-8'))
objects = {obj.name: obj for obj in bpy.data.objects if obj.type == 'MESH'}
if set(objects) != set(PIVOTS):
    raise RuntimeError(f'Pecas inesperadas: {set(objects)}')
for name, obj in objects.items():
    if any(abs(obj.matrix_world.translation[i] - PIVOTS[name]['pivot'][i]) > 1e-5
           for i in range(3)):
        raise RuntimeError(f'Pivo incorreto em {name}')

texture_paths = {}


def image_material(name, image):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    shader = material.node_tree.nodes.get('Principled BSDF')
    shader.inputs['Base Color'].default_value = (1, 1, 1, 1)
    shader.inputs['Roughness'].default_value = 0.6
    node = material.node_tree.nodes.new('ShaderNodeTexImage')
    node.image = image
    material.node_tree.links.new(node.outputs['Color'], shader.inputs['Base Color'])
    return material


def bake_mug(obj):
    # Grava a cor procedural da caneca em uma UV sem sobreposicoes.
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    for modifier in obj.modifiers:
        modifier.show_render = False
        modifier.show_viewport = False
    uv = obj.data.uv_layers.new(name='VART_Color')
    obj.data.uv_layers.active = uv
    uv.active_render = True
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=math.radians(66), island_margin=0.02)
    bpy.ops.object.mode_set(mode='OBJECT')

    image = bpy.data.images.new(obj.name + '_Cor', width=1024, height=1024, alpha=False)
    for slot in obj.material_slots:
        material = slot.material
        target = material.node_tree.nodes.new('ShaderNodeTexImage')
        target.image = image
        material.node_tree.nodes.active = target
    bpy.context.scene.render.engine = 'CYCLES'
    bpy.context.scene.cycles.device = 'CPU'
    bpy.context.scene.cycles.samples = 1
    bpy.ops.object.bake(type='DIFFUSE', pass_filter={'COLOR'}, margin=8, use_clear=True)
    path = TEXTURES / (obj.name + '_Cor.png')
    image.filepath_raw = str(path)
    image.file_format = 'PNG'
    image.save()
    material = image_material(obj.name + '_Cor', image)
    obj.data.materials.clear()
    obj.data.materials.append(material)
    for polygon in obj.data.polygons:
        polygon.material_index = 0
    texture_paths[material.name] = 'textures/' + path.name
    print('PINTURA', obj.name, path)


for name in ('Caneca_0', 'Caneca_1'):
    bake_mug(objects[name])

# Copia a pintura original do piso, sem alterar os pixels.
floor_image = bpy.data.images.get('Cha Viking | Piso XINAS')
if floor_image is None or floor_image.packed_file is None:
    raise RuntimeError('A copia Blender precisa conter a pintura XINAS empacotada.')
floor_path = TEXTURES / 'ChaViking_Piso_XINAS.png'
floor_path.write_bytes(bytes(floor_image.packed_file.data))
floor_image = bpy.data.images.load(str(floor_path), check_existing=False)
floor_material = image_material('Piso_XINAS', floor_image)
texture_paths[floor_material.name] = 'textures/' + floor_path.name

aliases = {
    'CV | Ferro pintado - azul petroleo': 'Azul_Petroleo',
    'CV | Ferro pintado - creme': 'Creme',
    'CV | Ferro pintado - coral': 'Coral',
    'CV | Piso metalico - Wave Swinger': 'Piso_Metalico',
    'CV | Aros de ferro escovado': 'Ferro',
    'CV | Rebites de aco': 'Rebites',
}
solids = {}
for obj in objects.values():
    if obj.name.startswith('Caneca_'):
        continue
    for slot in obj.material_slots:
        original = slot.material
        if original.name.startswith('CV | Piso XINAS'):
            slot.material = floor_material
            continue
        alias = next((value for prefix, value in aliases.items()
                      if original.name.startswith(prefix)), None)
        if alias is None:
            raise RuntimeError(f'Material nao previsto: {original.name}')
        if alias not in solids:
            shader = next(node for node in original.node_tree.nodes
                          if node.type == 'BSDF_PRINCIPLED')
            color = shader.inputs['Base Color'].default_value
            material = bpy.data.materials.new(alias)
            material.diffuse_color = color
            material.use_nodes = True
            material.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value = color
            solids[alias] = material
        slot.material = solids[alias]

worlds = {name: obj.matrix_world.copy() for name, obj in objects.items()}
parents = {name: obj.parent for name, obj in objects.items()}
try:
    # Exporta cada peca relativa ao pivo salvo no Blender.
    for obj in objects.values():
        obj.parent = None
    bpy.context.view_layer.update()
    for name, obj in objects.items():
        matrix = worlds[name].copy()
        matrix.translation = (0, 0, 0)
        obj.matrix_world = matrix
    bpy.context.view_layer.update()
    bpy.ops.object.select_all(action='DESELECT')
    for obj in objects.values():
        obj.select_set(True)
    bpy.ops.wm.obj_export(
        filepath=str(OUTPUT), export_selected_objects=True,
        apply_modifiers=False, export_triangulated_mesh=True,
        export_materials=True, path_mode='RELATIVE',
        forward_axis='Y', up_axis='Z',
    )
finally:
    for name, obj in objects.items():
        obj.parent = parents[name]
        obj.matrix_world = worlds[name]
    bpy.context.view_layer.update()

# O leitor VART herda a textura anterior; escreve os materiais com mapa por ultimo.
sections = []
for line in OUTPUT.with_suffix('.mtl').read_text(encoding='utf-8').splitlines():
    if line.startswith('newmtl '):
        sections.append((line[7:], []))
    elif sections:
        sections[-1][1].append(line)
lines = ['# Cha Viking: materiais preparados para o leitor VART.', '']
for name, section in sorted(sections, key=lambda item: item[0] in texture_paths):
    lines.append('newmtl ' + name)
    if name in texture_paths:
        lines.extend(('Kd 1.000000 1.000000 1.000000', 'Ka 0.300000 0.300000 0.300000'))
    for line in section:
        if not line or line.startswith(('Ka ', 'map_', 'bump ')):
            continue
        if line.startswith('Kd '):
            if name in texture_paths:
                continue
            lines.append(line)
            color = [float(value) * 0.3 for value in line.split()[1:4]]
            lines.append('Ka ' + ' '.join(f'{value:.6f}' for value in color))
        elif line.startswith('Ks '):
            lines.append('Ks 0.150000 0.150000 0.150000')
        else:
            lines.append(line)
    if name in texture_paths:
        lines.append('map_Kd ' + texture_paths[name])
    lines.append('')
OUTPUT.with_suffix('.mtl').write_text('\n'.join(lines), encoding='utf-8')
print('Exportado:', OUTPUT)
