# 脚本功能: 遍历当前文件夹下所有xxx.obj模型,统计Obj模型的各项数据
import trimesh
import os

def summarize_obj_files_trimesh(directory_path):
    # 遍历指定目录下的所有文件
    for filename in os.listdir(directory_path):
        if filename.endswith(".obj"):
            input_obj_path = os.path.join(directory_path, filename)
            # 使用 Trimesh 读取 OBJ 文件
            mesh = trimesh.load(input_obj_path, force='mesh')
            
            if mesh.is_empty:
                print(f"{filename}: 文件无法读取或为空")
            else:
                print(f"{filename}:")
                print(f"  顶点数: {len(mesh.vertices)}")
                print(f"  三角形面片数: {len(mesh.faces)}\n")

# 输入目录路径
directory_path = './'
# 调用函数处理当前目录
summarize_obj_files_trimesh(directory_path)





