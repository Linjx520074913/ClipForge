模块目录	职责简述
core	
renderer	GPU 渲染器 + 多滤镜组合，提供 setFilters/renderFrame 接口
exporter	导出帧为视频/图片序列，封装 mp4/webm
types	所有接口定义，便于 IDE 类型提示和隔离
utils	时间换算、日志封装、任务队列等辅助功能