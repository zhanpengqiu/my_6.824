# 基于CPP实现6.824相关项目
## 1.相关技术
  grpc：grpc进行主机间的信息交互
## 2.相关章节介绍
相关接口以及实现均参考相关论文实现
### 2.1 MapReduce
  这一部分的内容还未全部完成，但完成了大部分的功能。
  1. 实现了Coordinator分配任务
  2. 实现了Worker完成相关人物
  3. 测试了Worker掉线的情况。
  4. 后续补充测试日志......
### 2.2 Raft
  这一部分内容也未全部完成，但完成了大部分的功能。
  1. 完成了领导选举的功能
  2. 完成了日志同步的功能
  3. 完成了日志持久化的功能
  4. 实现了apply的部分功能
  5. 完成快照制作的功能
  6. 完成了从机快照落后能从主机获取快照的功能
  7. 测试了领导超时的情况（良好）
  8. 测试了其他主机超时的情况（良好）
  9. 测试了requestvote由于网络延迟的延迟收到、报文不回复的情况（良好）
  10. 测试了appendentries由于网络延迟收到、报文不回复的情况 （良好）
  11. 测试了10h，日志及状态机记录正常
  10. 后续补充测试日志以及相关测试记录...
### 2.3 kvsrv
  搭建部分框架
  1. 完成框架部分编写
  2. 根据框架补充内容，可能会有部分框架内容进行改变
  3. 待完成................
## 自问自答
q：为什么用cpp实现，用go不好吗？
a: 主要还是想锻炼一下自己吧，多学习学习cpp的一些特性，以及相关编程库。