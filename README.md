# 实验说明

请 fork 此 `repo` 到自己的仓库下，随后在自己的仓库中完成实验，请确保自己的 `repo` 为 `Private` 。

### 实验环境

推荐使用 Linux 环境，可参考[环境配置](./Documentations/environments.md)。

其他的具体搭建要求，可参考[实验操作指南](./%E7%BC%96%E8%AF%91%E5%8E%9F%E7%90%86%E5%AE%9E%E9%AA%8Cgitee%E5%B9%B3%E5%8F%B0%E6%8C%87%E5%8D%97.md)

### 目前已布置的实验:
*   [lab1](./Documentations/lab1/README.md)
    *   DDL : 2021年11月5日 23:59
*   [lab2](./Documentations/lab2/README.md)
    *   DDL : 待定
*   [lab3](./Documentations/lab3/README.md)
    *   DDL : 待定
*   [lab4](./Documentations/lab4/README.md)
    *   DDL : 待定
*   [lab5](./Documentations/lab5/README.md)
    *   DDL : 待定

### git 的使用

推荐同学们通过搜索引擎和相互交流学习掌握git这个工具。

后续可能也会更新一些教程和文档。

### 有关实验项目的疑问

推荐在 issues 新建，并按照 markdown 格式进行编写。

需要注明：

`问题范围`：如 `lab 1`，`项目的同步`等

`问题简述`： 问题的主要描述，如：实验代码运行时报错；必要时可提供截图

`问题详细描述`：使用的平台，问题的触发方式，自己尝试的操作等；必要时可提供截图

`其他信息`：其他可用的信息，例如：与个人有关，可以提供下联系方式（推荐邮箱，注意保护个人隐私）

有其他问题也可以发送邮件至 shibc@hnu.edu.cn。

### FAQ: How to merge upstream remote branches

In brief, you need another alias for upstream repository (we assume you are now in your local copy of forked repository on Gitlab):
```
(shell) $ git remote add upstream [project url]
```
Then try to merge remote commits to your local repository:
```
(shell) $ git pull upstream master
```
Then synchronize changes to your forked remote repository:
```
(shell) $ git push origin master
```

### 致谢

本实验项目参考了 USTC 2020年秋季学期编译原理实验项目，特此感谢。
