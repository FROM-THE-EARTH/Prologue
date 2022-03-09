# Prologue

単段 / 多段式ロケットの軌道解析シミュレーターです。

出力ファイルは審査書書きやすいようにしてます。

:warning: macOS 版は Apple Silicon には対応していません。

## Download

Latest Update: 2022/03/09

Version: 1.4.2

[**Download**](https://github.com/FROM-THE-EARTH/Prologue/releases/latest)

## 利用方法

1. Gnuplot を導入

   - **Windows**: [ここ](https://sourceforge.net/projects/gnuplot/files/gnuplot/5.2.8/)からインストール<br>
     gnuplot のデフォルト出力形式の選択で wxt を選択する
   - **macOS**: [Homebrew](https://brew.sh/index_ja)をインストール後、ターミナルで `brew install gnuplot`
   - **Linux**: ターミナルで `sudo apt install gnuplot`

2. ダウンロードした zip ファイルを展開

3. [各パラメータの入力形式](https://github.com/FROM-THE-EARTH/Prologue/blob/master/docs/INPUT.md)に沿ってファイルを作成する

4. `Prologue` を起動<br>
   macOS, Linux の場合はターミナル上でのみ実行可能

5. 指示に従ってシミュレーションを行う

6. `result/`フォルダ下に結果が格納される

**\* ファイル名に全角を使用すると不具合が生じる可能性があります**

## For Developers

必ず[これ](https://github.com/FROM-THE-EARTH/Prologue/blob/master/docs/DEVELOPMENT.md)を読んでください。

やることがなかったら [Issues](https://github.com/FROM-THE-EARTH/Prologue/issues) にあるものを消化するといいと思います。

## 変更履歴

### v1.4.2

- 磁気偏角が結果の緯度経度に対して考慮されていなかった問題を修正
- サンプルの JSON の`environment.place`を`nosiro_sea`に変更

### v1.4.1

- 能代陸・伊豆陸・伊豆海マップに磁気偏角を適用
- 結果の緯度経度がどのマップかに関わらず能代海基準になっていた問題を修正
- `spec_single_json` の`info.NAME`の値を`sample_multi`から`sample_single`に修正

### v1.4.0

- macOS, Linux に対応<br>
  macOS 版は Apple Silicon には対応していません。

### v1.3.1

- エンジン推力の線形補間アルゴリズムを修正

### v1.3.0

- 詳細解析モードにおいて、各ステップでの主要な値を出力するように変更

<details>
<summary>過去の変更履歴</summary>

### v1.2.1

- 伊豆陸マップを追加

### v1.2.0

- 圧力中心傾斜、抗力係数傾斜を計算に含むように変更
- 機体速度に対する圧力中心（傾斜），抗力係数（傾斜），法線力係数　を入力するを追加(input/airspeed_param/\*\*.csv として保存する。指定しない場合は諸元 JSON の定数を用いる。傾斜は指定されていない場合 0 となる。)
- 機体諸元 json ファイルに有効でない値がある場合にエラーを出力するように変更
- 伊豆海の緯度経度情報を設定

### v1.1.1

- 抗力係数の向きを修正
- 風向風速ファイルにヘッダを追加

### v1.1.0

- 伊豆海マップを追加

### v1.0.1

- 弾道シミュレーションモードで最高高度時刻が取得できない不具合を修正

### v1.0.0

- リリース

</details>
