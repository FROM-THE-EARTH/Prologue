# Prologue
  単段/多段式ロケットの軌道解析用シミュレーターです。

# Download
  Latest Update: 2020/11/11

  Version: 1.0.0

  [Download](https://raw.githubusercontent.com/FROM-THE-EARTH/Prologue/master/Prologue/Application/Prologue.zip)

# 利用方法
  1. [Gnuplot](https://sourceforge.net/projects/gnuplot/files/gnuplot/5.2.8/)を導入

  2. Prologue.zipを展開

  3. Prologue.exeを起動

  4. ロケットの諸元はinput/json/に、エンジンのスラストデータはinput/thrust/に、風向風速データはinput/wind/にそれぞれ保存する

  5. prologue.settings.jsonからアプリケーションの設定を行う

  6. 指示に従ってシミュレーションを行う

  7. result/フォルダ下に結果が格納される

# TODO
  - 複数パラシュートへの対応
  - パラシュート開傘ディレイへの対応
  - 能代海以外のマップの追加
  - 機軸方向ベクトルのグラフ化機能の追加

# For Developers
  必ず[これ](https://raw.githubusercontent.com/FROM-THE-EARTH/Prologue/master/docs/DEVELOP.md)を読んでください。