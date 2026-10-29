"""Exporta Helicoptero.blend para o projeto VART sem alterar o .blend.

Uso: blender -b Helicoptero.blend --python exportar_helicoptero.py
"""

from pathlib import Path

import bpy


DEST = Path(__file__).resolve().parent.parent / 'projetos' / 'helicopter'
DEST.mkdir(exist_ok=True)
OUTPUT = DEST / 'Helicoptero.obj'
EXPECTED = {
    'Helicopter Body', 'Main Rotor', 'Tail Rotor',
    'Landing Skids', 'Missiles', 'Cylinder',
}
TEXTURES = {
    'Base': 'Metal_Grill_005a_Base_Color.jpg',
    'Helicopter': 'Helicopter_Base_Color.png',
}

objects = {obj.name: obj for obj in bpy.data.objects if obj.type == 'MESH'}
if set(objects) != EXPECTED:
    raise RuntimeError(f'Pecas inesperadas: {set(objects)}')

# O OBJ do programa usa coordenadas locais; o C++ recoloca cada peca.
matrices = {name: obj.matrix_world.copy() for name, obj in objects.items()}
parents = {name: obj.parent for name, obj in objects.items()}
for obj in objects.values():
    obj.parent = None
for name, obj in objects.items():
    matrix = matrices[name].copy()
    matrix.translation = (0, 0, 0)
    obj.matrix_world = matrix
bpy.context.view_layer.update()

try:
    bpy.ops.object.select_all(action='DESELECT')
    for obj in objects.values():
        obj.select_set(True)
    bpy.ops.wm.obj_export(
        filepath=str(OUTPUT), export_selected_objects=True,
        export_triangulated_mesh=True, export_uv=True, export_normals=True,
        export_materials=True, path_mode='RELATIVE',
        forward_axis='NEGATIVE_Z', up_axis='Y',
    )
finally:
    for name, obj in objects.items():
        obj.parent = parents[name]
        obj.matrix_world = matrices[name]
    bpy.context.view_layer.update()

# Usa apenas mapas de cor, com nomes que o leitor MTL do VART aceita.
for material, filename in TEXTURES.items():
    image = next((img for img in bpy.data.images if Path(img.filepath.replace('\\', '/')).name == filename), None)
    if image is None:
        raise RuntimeError(f'Textura ausente: {filename}')
    target = DEST / filename
    if image.packed_file:
        target.write_bytes(bytes(image.packed_file.data))
    else:
        source = Path(bpy.path.abspath(image.filepath))
        if not source.is_file():
            raise RuntimeError(f'Textura ausente: {source}')
        target.write_bytes(source.read_bytes())

mtl = OUTPUT.with_suffix('.mtl')
lines = []
material = None
for line in mtl.read_text(encoding='utf-8').splitlines():
    if line.startswith('newmtl '):
        material = line[7:]
    if line.startswith(('map_', 'bump ')):
        continue
    lines.append(line)
    if line.startswith('illum ') and material in TEXTURES:
        lines.append('map_Kd ' + TEXTURES[material])
mtl.write_text('\n'.join(lines) + '\n', encoding='utf-8')
print('Exportado:', OUTPUT)