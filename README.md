# Prologue

単段 / 多段式ハイブリッドロケットの軌道シミュレーターです。

## Download

Latest Update: 2025/07/07

Version: 1.9.8.4

[**Download**](https://github.com/kuma003/Prologue/releases/latest)

[過去のバージョン、更新履歴](https://github.com/kuma003/Prologue/releases)

## 利用方法

1. 事前準備

   - Gnuplot の導入

     - **Windows**: [ここ](https://sourceforge.net/projects/gnuplot/files/gnuplot/5.2.8/)からインストール<br>
       gnuplot のデフォルト出力形式の選択で **wxt** を選択

     - **macOS**: [Homebrew](https://brew.sh/index_ja)をインストール後、ターミナルで `brew install gnuplot`

     - **Linux**: ターミナルで `sudo apt install gnuplot`

   - その他
     - **Linux**: `--open-result` オプション指定時にエラーが発生する場合があるため以下を実行
       ```sh
       sudo apt install samba-common-bin
       sudo mkdir -p /var/lib/samba/usershares
       ```

2. [ダウンロード](https://github.com/FROM-THE-EARTH/Prologue/releases/latest)した Prologue の zip ファイルを展開

3. [各パラメータの入力形式](https://github.com/FROM-THE-EARTH/Prologue/blob/master/docs/INPUT.md)に沿ってファイルを作成する

4. Prologue を起動、指示に従ってシミュレーションを実行<br>
   macOS, Linux の場合はターミナル上でのみ実行可能

5. `result/` フォルダ下に結果が格納される

## コマンドラインオプション

ターミナルで実行時、`./Prologue --command`のようにして以下のオプションを指定できます。

- `--no-save`: 結果を保存しない
- `--no-plot`: 結果をプロットしない
- `--dry-run`: 結果を保存もプロットしない（`--no-save --no-plot`と同値）
- `--open-result`: シミュレーション実行後に結果のフォルダをエクスプローラーなどで開く

## For Developers

必ず[これ](https://github.com/FROM-THE-EARTH/Prologue/blob/master/docs/DEVELOPMENT.md)を読んでください。

やることがなかったら [Issues](https://github.com/FROM-THE-EARTH/Prologue/issues) にあるものを消化するといいと思います。
