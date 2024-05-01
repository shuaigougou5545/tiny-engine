# Shader 参数说明

1）顶点数据attribute：a_大驼峰

| 变量名     | location | 类型 | 含义             |
| ---------- | -------- | ---- | ---------------- |
| a_Pos      | 0        | vec3 | 局部空间顶点位置 |
| a_Normal   | 1        | vec3 | 局部空间法向量   |
| a_TexCoord | 2        | vec2 | 纹理uv           |

2）通用Uniform变量：u_大驼峰

| 变量名          | 类型      | 含义               |
| --------------- | --------- | ------------------ |
| u_Model         | mat4      | model              |
| u_View          | mat4      | view               |
| u_Projection    | mat4      | projection         |
|                 |           |                    |
| u_CameraPos     | vec3      | 世界空间摄像机位置 |
|                 |           |                    |
| u_LightPos      | vec3      | 世界空间光源位置   |
| u_LightStrength | vec3      | 光源颜色           |
|                 |           |                    |
| u_Texture0      | sampler2D | 默认纹理           |

3）着色器间varying变量传递：v_大驼峰

4）着色器内部：小驼峰(后缀:隶属空间)

```
normalW: 世界空间法向量
posV: 观察空间物体位置
```

