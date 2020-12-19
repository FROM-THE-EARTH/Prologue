# Prologue
  単段/多段式ロケットの軌道解析シミュレーターです。

  出力ファイルは審査書書きやすいようにしてます。

# Download
  Latest Update: 2020/11/23

  Version: 1.1.0

  [**Download**](https://raw.githubusercontent.com/FROM-THE-EARTH/Prologue/master/Prologue/Application/Prologue.zip)

# 利用方法
  1. [Gnuplot](https://sourceforge.net/projects/gnuplot/files/gnuplot/5.2.8/)を導入

  2. Prologue.zipを展開

  3. ロケットの諸元はinput/json/に、エンジンのスラストデータはinput/thrust/に、風向風速データはinput/wind/にそれぞれ保存する

  4. prologue.settings.jsonからアプリケーションの設定を行う

  5. Prologue.exeを起動(コマンドライン上で実行も可能)

  6. 指示に従ってシミュレーションを行う

  7. result/フォルダ下に結果が格納される

# TODO
  - 複数パラシュートへの対応
  - パラシュート開傘ディレイへの対応
  - 能代海以外のマップの追加
  - 機軸方向ベクトルのグラフ化機能の追加
  - 諸元JSONファイルの整理、できればExcelから直接読み込めるようなフォーマットの作成([C++用Excelライブラリ](https://github.com/troldal/OpenXLSX))

# For Developers
  必ず[これ](https://github.com/FROM-THE-EARTH/Prologue/blob/master/docs/DEVELOP.md)を読んでください。

# 変更履歴

## v1.1.1
- 抗力係数の向きを修正
- 風向風速ファイルにヘッダを追加

## v1.1.0
- 伊豆海マップを追加

## v1.0.1
- 弾道シミュレーションモードで最高高度時刻が取得できない不具合を修正

## v1.0.0
- リリース
