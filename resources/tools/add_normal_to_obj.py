# 脚本功能: 遍历当前文件夹下所有xxx.obj模型，为其计算法线并生成新的normals_xxx.obj

import open3d as o3d
import os
import time

# 设置日志级别为错误，这将只显示错误信息
o3d.utility.set_verbosity_level(o3d.utility.VerbosityLevel.Error)

def add_normals_to_obj(input_obj_path, output_obj_path):
    # 读取 OBJ 文件
    mesh = o3d.io.read_triangle_mesh(input_obj_path)
    
    # 开始计时
    start_time = time.time()
    print(f"正在计算 {input_obj_path} 的顶点法线数据")

    # 计算顶点法线
    mesh.compute_vertex_normals()

    # 保存 OBJ 文件，包括法线信息
    o3d.io.write_triangle_mesh(output_obj_path, mesh, write_vertex_normals=True)
    
    # 计算并打印处理时间
    elapsed_time = time.time() - start_time
    print(f"计算 {input_obj_path} 耗时 in {elapsed_time:.2f} seconds")

def process_directory(directory_path):
    # 获取当前目录下所有文件
    for filename in os.listdir(directory_path):
        # 检查文件是否是 OBJ 文件
        if filename.endswith(".obj"):
            input_obj_path = os.path.join(directory_path, filename)
            output_obj_path = os.path.join(directory_path, "normals_" + filename)
            # 调用函数处理每个 OBJ 文件
            add_normals_to_obj(input_obj_path, output_obj_path)
            print(f"Processed {filename} into {output_obj_path}")

# 输入目录路径
directory_path = './'

# 调用函数
process_directory(directory_path)
