# ソースコードについて
## カテゴリ無し
**Prologue (cpp)**<br>
プログラムの起動部分。int main()。バージョン変更時はここをいじる。

**Algorithm (h, cpp)**<br>
Algorithm名前空間。プログラム全体で使用するかもしれない汎用的な関数を定義する。

**Constant(h)**<br>
Constant名前空間。円周率、絶対零度、気体定数などの各種定数を定義する。

## Simulatorカテゴリ
ソフトウェアのインターフェース的な部分。
**Simulator(h, cpp)**<br>
Simulatorクラス。コマンドライン上で設定を行い、諸元を読み込み、Solverクラス(後述)に渡す。

**ResultSaver(h, cpp)**<br>
ResultSaver名前空間。解析結果を定められたフォーマットで保存するためのAPI的な。

**Gnuplot(h, cpp)**<br>
Gnuplot名前空間。Gnuplotに対してコマンドを送信し、解析結果の表示、保存等を行う。

**MapFeature(h, cpp)**<br>
各マップの緯度経度情報を定義し、距離の計算などを行う。あまり実装が美しくない。

**CommandLine(h, cpp)**<br>
CommandLine名前空間。エラーはE、警告はW、情報はIを出力の全ての文頭につけ、統一された出力を提供する。解析終了後のコマンド実行も担う。

## Solverカテゴリ
軌道解析部分。
**Solver(h, cpp)**<br>
Solverクラス等。Simulatorクラスから提供された諸元等を用いて、1回の打ち上げ全ての軌道計算を実行する。マルチスレッド時は1つのスレッドに付き1つのSolverが実行されている。Solverクラス及びその内部で実行される全ての計算はスレッドセーフでなければならない。

**Air(h, cpp)**<br>
Airクラス。機体の高度を毎ステップ受け取り、その高度に応じて風向風速重力等を計算、提供する。1Solverに1Airのみが存在する。

**Rocket(h)**<br>
Rocket構造体。ある時点における機体の重量、場所、速度、クォータニオン、経過時間、パラシュート等の状態を保持する。

## RocketSpecカテゴリ
機体諸元。
**RocketSpec(h)**<br>
機体の静的パラメータ。機体諸元, 推力情報, 対気速度別パラメータ等を保持する。

**Engine(h, cpp)**<br>
Engineクラス。推力履歴を読み込み、ある秒数における推力を計算、提供する。

**AirspeedParam(h, cpp)**<br>
AirspeedParamクラス。機体の対気速度に応じて変化するパラメータを計算、提供する。

## Jsonカテゴリ
Jsonファイルの読み込み、管理等。
**AppSettng(h, cpp)**<br>
ソフトウェア自体の設定を行う。prologue.settings.jsonを読み込む。

**RocketSpecReader(h, cpp)**<br>
RocketSpecReader名前空間。機体諸元を読み込む。

**JsonUtils(h, cpp)**<br>
JsonUtil名前空間。Jsonファイルの読み込み時に使用する関数をまとめて定義している。

## Typeカテゴリ
型定義。
**Vector3D(h, cpp)**<br>
Vector3D構造体。ベクトル(x, y, z)の計算を行う。

**Quaternion(h, cpp)**<br>
Quaternion構造体。クォータニオン(x, y, z, w)の計算を行う。なんかうまくいってるけどあまり自信ない。
