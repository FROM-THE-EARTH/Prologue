# For Developers
  開発者向けドキュメント

  物理とか風とかシミュレーション内部ついては[ここ](https://github.com/FROM-THE-EARTH/Prologue/blob/master/docs/DYNAMICS.md)に書いてます。

# 開発環境等
  - **使用言語** : C++17
  - **開発環境** : Visual Studio 2019 (導入推奨)
  - **ライブラリ** : boost 1.71.0 (リポジトリに同包済み)
  - **外部ソフトウェア** : Gnuplot 5.2.8
  - **OS** : Windows10

# 開発時の注意事項
 - **元から入ってるファイルは削除しないでください。**
 - **プロジェクトの設定はいじらないでください。**
 - **リリース用zipファイルは、PowerShellコマンドによってReleaseビルド時に自動で生成されます。生成後にcommit, pushすれば当然そのファイルがGitHub上でリリースされます。**

# プロジェクトのディレクトリ構成
```bash
 Prologue/
         ├Prologue/               #プロジェクトディレクトリ
         |        ├Application/   #ビルド時の作業ディレクトリ及びリリース用ファイル群
         |        ├Intermediate/  #ビルド時の中間ファイル郡
         |        ├libraries/     #外部ライブラリ用ディレクトリ
         |        └src/           #ソースファイル郡
         └docs/                   #ドキュメント
```

# ソフトウェア開発にあたって
## バージョン管理について
**基本のバージョン表記はx.y.zです。**

- xは大幅な仕様変更があった場合にインクリメントします。(以前のバージョンとの互換性が無くなるくらい)

- yは互換性はあるが、仕様にそれなりの変更があった場合にインクリメントします。

- zはバグ修正や軽微なアップデートなどを行った場合にインクリメントします。

## コーディング
### 一般的なこと
- using namespaceしない
- const変数, constexpr変数, 名前空間内変数/メソッド以外でグローバル変数を使わない
- 初期化以外の書き換えが行われない場合、なるべくconstを付ける
- 変数名に単純にaとかbとかvalueとかnumとかつけない(一瞬使うだけならOK)
- 行を詰めて書かない
- 適宜コメントを追加する(何をしているかがわかるだけでなく、一般的なテキストエディタならコメント用の色がつくはずなのでひと目見てここで処理内容が変わるってことがわかります)

### 保守性/可用性の維持
常に今後の開発を見据えたコードを書きましょう。それなりに規模の大きいプロジェクトなのでちゃんとしないと後輩が死にます。
- 新たに機能を追加する場合は、既存のclassやnamespaceに突っ込むのではなく分離できないかを常に考える
- 1つのオブジェクトに対していくつかの変数が存在するときはstructやclassを使うことを検討する(例：pos, velocity, timeを分割するのではなく、struct Rocket内で定義する)

### コーディングの統一(保守性関連)
- 基本的にはキャメルケースを利用する
- class内staticメソッドや名前空間内の関数にはパスカルケースを利用する
- json内のkey名などと関連があるような変数に関してはスネークケースを用いても良い
- 変数の意味を確実に伝えなければならない場合、例外的に添字を付けるなどをしてもよい(force_b, moment_bなど)
- class内private変数には変数名の末尾に_(アンダーバー)を付ける(std::string filename_のように)

### 処理関連
常にこのアルゴリズムは妥当なのか、もっと速度を伸ばせないか考えましょう。<br>
速度を上げることはつまりPCへの負担を軽減することでもあります。<br>
シミュレーションは同じ処理を何度も繰り返すことが多いです。ちょっとしたことでも塵が積もれば山となります。
- ある関数がstructやclassオブジェクトを引数とするとき、必ずconst参照渡し(const Type& type)または、関数内で書き換える場合はポインタ渡し(Type *type)とする
- 何度もpush_backするstd::vectorがある場合、事前にreserveできないかを考える
- 無駄に何度もしてるような処理があればそこだけ分離して1度だけ実行できないか考える