---
title: hexo+github.io+Travis
date: 2017-08-03 00:46:57
tags:
- 小技巧
- github
---

## Hexo + github + Travis

github page 大家都知道是github免费给我们使用的网站服务主机，可以用来展示一个静态的博客应用。Hexo恰好是一个静态博客应用的生成工具，他们结合在一起就是否方便我们编辑控制生成博客网站了。但是如果加入Travis就更加完美了，Travis是针对github项目开发的持续集成工具，你只要提个PR一下编译发布的动作都可以通过事先定义的脚本自动执行，是不是很棒

![1](/images/1.gif)

教程网上都有，大家随便找找就好，比如：
<!-- more -->
* https://hexo.io/zh-cn/docs/index.html
* https://www.liaoxuefeng.com/article/0014631488240837e3633d3d180476cb684ba7c10fda6f6000

我就遇到一个坑，^_^

就是在travis CI脚本最后执行hexo deploy把编译得到的静态web应用发布到github pages上的时候需要设置github的公钥，或者使用用户名密码通过https上传。

```yaml
language: node_js
node_js:
  - "6"
install:
  - npm install
script:
  - hexo clean
  - hexo generate
  - echo "https://hhhdddwww%40163.com:"$GIT_PWD"@github.com" >> ~/.git-credentials
  - git config --global credential.helper store
  - hexo deploy
```

我调用了一下发现SSH公钥没有办法设置到Travis CI上，所以只能采用用户名密码的方式，但是这样一来因为我的脚本是公开的一个项目，那密码就会暴露了，幸好Travis CI上可以设置私密的环境变量，如上面脚本中`GIT_PWD` ，这样一来问题就解决了，大家也快来试试吧，虽然好像CI并不需要。。。。。。但是能撞壁我们尽量不沉默，哈哈

![1](/images/2.jpeg)

