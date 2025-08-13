# 总项目名
- ClipForge 作为总项目
- ClipForgeCore 底层核心 C++
  功能：音视频编解码
  目标：高性能
- ClipForgeRuntime Rust 运行时，调用 ClipForgeCore
  功能：调用 C++ 底层接口，管理帧流等