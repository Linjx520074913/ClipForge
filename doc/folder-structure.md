---
sidebar: auto
---

# 项目文件夹结构说明

本项目采用模块化设计，代码和资源按照功能和职责划分为多个文件夹。以下是主要目录的说明：

- 根目录（root）
  - `src/`：前端源码（Vue 组件、业务逻辑）
  - `src-express/`：后端源码（Node.js + Express）
  - `src-tauri/`：rust 源码
  - `shared/`：前后端共用的类型定义、接口、Schema
  - `docs/`：项目文档，使用 VitePress 管理
  - `public/`：静态资源（图片、字体、图标等）
  - 
  - 其他配置文件：
    - `package.json`：依赖和脚本配置
    - `tsconfig.json`：TypeScript 配置
