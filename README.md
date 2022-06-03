# Prologue

単段 / 多段式ロケットの軌道解析シミュレーターです。

## Download

Latest Update: 2022/06/03

Version: 1.8.0

[**Download**](https://github.com/FROM-THE-EARTH/Prologue/releases/latest)

[過去のバージョン、更新履歴](https://github.com/FROM-THE-EARTH/Prologue/releases)

## 利用方法

1. Gnuplot を導入

   - **Windows**: [ここ](https://sourceforge.net/projects/gnuplot/files/gnuplot/5.2.8/)からインストール<br>
     gnuplot のデフォルト出力形式の選択で **wxt** を選択する
   - **macOS**: [Homebrew](https://brew.sh/index_ja)をインストール後、ターミナルで `brew install gnuplot`
   - **Linux**: ターミナルで `sudo apt install gnuplot`

2. ダウンロードした zip ファイルを展開

3. [各パラメータの入力形式](https://github.com/FROM-THE-EARTH/Prologue/blob/master/docs/INPUT.md)に沿ってファイルを作成する

4. `Prologue` を起動<br>
   macOS, Linux の場合はターミナル上でのみ実行可能

5. 指示に従ってシミュレーションを行う

6. `result/` フォルダ下に結果が格納される

## For Developers

必ず[これ](https://github.com/FROM-THE-EARTH/Prologue/blob/master/docs/DEVELOPMENT.md)を読んでください。

やることがなかったら [Issues](https://github.com/FROM-THE-EARTH/Prologue/issues) にあるものを消化するといいと思います。
