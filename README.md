# Prologue
  単段/多段式ロケットの軌道解析シミュレーターです。

  出力ファイルは審査書書きやすいようにしてます。

# Download
  Latest Update: 2021/08/14

  Version: 1.3.1

  [**Download**](https://raw.githubusercontent.com/FROM-THE-EARTH/Prologue/master/Prologue/Application/Prologue.zip)

# 利用方法
  1. [Gnuplot](https://sourceforge.net/projects/gnuplot/files/gnuplot/5.2.8/)を導入(gnuplotのデフォルト出力形式の選択でwxtを選択する)

  2. Prologue.zipを展開

  3. [各パラメータの入力形式](https://github.com/FROM-THE-EARTH/Prologue/blob/master/docs/INPUT.md)に沿ってファイルを作成する

  4. Prologue.exeを起動(コマンドライン上で実行も可能)

  5. 指示に従ってシミュレーションを行う

  6. result/フォルダ下に結果が格納される
  
  ***ファイル名に全角を使用すると不具合が生じる可能性があります**
  
  **linuxでは実行不可、macの場合はwineやbootcampを利用する**

# For Developers
  必ず[これ](https://github.com/FROM-THE-EARTH/Prologue/blob/master/docs/DEVELOP.md)を読んでください。

  やることがなかったらIssuesにあるものを消化するといいと思います。
  
  あとコード結構汚いと思うのでこれをきれいにしてもらえるとgood.

# 変更履歴
## v1.3.1
 - エンジン推力の線形補間アルゴリズムを修正

## v1.3.0
 - 詳細解析モードにおいて、各ステップでの主要な値を出力するように変更

## v1.2.1
 - 伊豆陸マップを追加

<details>
<summary>過去の変更履歴</summary>

## v1.2.0
- 圧力中心傾斜、抗力係数傾斜を計算に含むように変更
- 機体速度に対する圧力中心（傾斜），抗力係数（傾斜），法線力係数　を入力するを追加(input/airspeed_param/**.csvとして保存する。指定しない場合は諸元JSONの定数を用いる。傾斜は指定されていない場合0となる。)
- 機体諸元jsonファイルに有効でない値がある場合にエラーを出力するように変更
- 伊豆海の緯度経度情報を設定

## v1.1.1
- 抗力係数の向きを修正
- 風向風速ファイルにヘッダを追加

## v1.1.0
- 伊豆海マップを追加

## v1.0.1
- 弾道シミュレーションモードで最高高度時刻が取得できない不具合を修正

## v1.0.0
- リリース

</details>
