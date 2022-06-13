# input 形式

シミュレーションを行う際に設定する以下の項目について、ファイルの形式を示します。

- シミュレータの設定 [サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/prologue.settings.json)
- 機体諸元 [単段サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/json/spec_single.json) [多段サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/json/spec_multi.json)
- エンジン [サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/thrust/Sample_G40-4W.txt)
- 風向風速 [サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/wind/sample.csv)
- 機体速度 vs Cp, Cp_a, Cd_i,Cd_f, Cd_a2, Cna [サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/aero_coef/sample.csv)

## シミュレータの設定

シミュレータの設定を行います。<br>
**保存ファイル名**: `prologue.settings.json`<br>
**拡張子**: `json`<br>
**形式**:

```
{
  "processing": {
    "multi_thread": false // マルチスレッドによる処理の高速化。Scatterモードでのみ有効。
    "multi_thread_count": 4 // スレッド数
  },

  "simulation": {
    "dt": 0.001, // １ステップで経過する時間。小さければより精度が上がるが計算に時間がかかる。
    "detect_peak_threshold": 15.0, // 頂点検知によるパラシュート開傘時、最高高度から何m落下したら開傘するかどうか[m]

    "scatter": {
      "wind_speed_min": 0.0, // Scatterモード時、シミュレーション対象の風速の最小値[m]
      "wind_speed_max": 7.0, // Scatterモード時、シミュレーション対象の風速の最大値[m]
      "wind_dir_interval": 30.0 // Scatterモード時、風向を何度ずつ変更してシミュレーションするか[deg]
    }
  },

  "result": {
    "precision": 8, // 結果出力時の数値の小数点以下の桁数
    "step_save_interval": 10 // 何回のステップ毎に結果を保存するか（出力ファイルが巨大になるのを防ぐため）
  },

  "wind_model": {
    "power_constant": 7.0, // べき法則の係数
    "power_low_base_alt": 2.0, // べき法則の基準高度[m]
    "type": "real", // 使用する風モデル　real, original, only_powerlow, no_wind
    "realdata_filename": "wind_data_template.csv" // 風向風速データのファイル名。typeがrealの場合のみ有効。
  }
}
```

## 機体諸元

機体諸元を記述します。<br>
**保存フォルダ**: `input/json/`<br>
**拡張子**: `json`<br>
**形式**:

```
{
	"rocket1": {
		"ref_len": "Rocket length[m]",
		"diam": "Rocket diameter[m]",
		"CGlen_i": "Initial CG place from nose[m] 重心",
		"CGlen_f": "Final CG place from nose[m]　重心",
		"mass_i": "Initial mass[kg] 乾燥質量+酸化剤質量」",
		"mass_f": "Final mass[kg] 乾燥質量",
		"Iyz_i": "Initial inertia moment of rolling[kg*m^2]　ピッチ/ヨー",
		"Iyz_f": "Final inertia moment of rolling[kg*m^2] ピッチ/ヨー",
		"Cmq": "Pitch damping moment coefficient ピッチ減衰モーメント係数",
		"vel_1st": "Terminal velocity of 1st parachute",
		"op_type_1st": "0:detect-peak, 1:fixed-time 2:time-from-detect-peak",
		"op_time_1st": "open time",
		"delay_time_1st": "パラシュートが開くのにかかる時間",
		"motor_file": "エンジンの推力履歴ファイル名　分離後1段目などの場合は空白にする",

		"CPlen": "CP place from nose[m] 圧力中心(aero_coef_fileがある場合は無効)",
		"Cp_alpha": "圧力中心傾斜[m/rad] from nose(aero_coef_fileがある場合は無効)",
		"Cd_i": "燃焼終了前の Drag coefficient 抗力係数(aero_coef_fileがある場合は無効)",
		"Cd_f": "燃焼終了後の Drag coefficient 抗力係数(aero_coef_fileがある場合は無効)",
		"Cd_alpha2": "抗力係数傾斜[/rad^2](aero_coef_fileがある場合は無効)",
		"Cna": "Normal force coefficient 法線力整数(aero_coef_fileがある場合は無効)",
		"aero_coef_file": "圧力中心（傾斜），抗力係数（傾斜），法線力係数 vs 機体速度　のcsvファイル名"
	},

	"rocket2": {
		"": "分離後下段。中身は上と同じ。不要な場合はrocket2を削除する。"
	},

	"rocket3": {
		"": "分離後上段。中身は上と同じ。不要な場合はrocket3を削除する。"
	},

	"environment": {
		"place": "マップ名称: nosiro_sea nosiro_land izu_sea izu_land",
		"magnetic_declination": "磁気偏角[deg] nullまたは数値（nullの場合はマップごとのデフォルト値を使用 DYNAMICS.md参照）"
		"rail_len": "ランチャレール長[m]",
		"rail_azi": "ランチャ方位角[degree]　北から右回り",
		"rail_elev": "ランチャ迎角[degree]"
	}
}
```

## エンジン

エンジンの推力を記述します。<br>
**保存フォルダ**: `input/thrust/`<br>
**拡張子**: `txt`<br>
**形式**: 時刻<半角スペース>推力

```
0 10
0.01 20
0.02 30
```

## 風向風速

実際の風向風速データを使用する場合、このファイルを作成し指定します。<br>
**保存フォルダ**: `input/wind/`<br>
**拡張子**: `csv`<br>
**形式**: 1 行目をヘッダとし、2 行目から値を入力する。

| geopotential_height | wind_speed[m/s] | wind_direction[deg](North:0, East:90) |
| ------------------- | --------------- | ------------------------------------- |
| 100                 | 4               | 100                                   |
| 200                 | 6               | 150                                   |
| 500                 | 6               | 180                                   |

## 機体速度 vs Cp, Cp_a, Cd_i, Cd_f, Cd_a2, Cna

機体速度に応じたパラメータを使用する場合、このファイルを作成し指定します。<br>
ファイルが存在しない、または指定されていない場合は機体諸元で指定された値を使用します。<br>
**保存フォルダ**: `input/aero_coef/`<br>
**拡張子**: `csv`<br>
**形式**: 1 行目をヘッダとし、2 行目から値を入力する。

| air_speed[m/s] | Cp_from_nose[m] | Cp_a[m/rad] | Cd_i | Cd_f | Cd_a2[/rad^2] | Cna  |
| -------------- | --------------- | ----------- | ---- | ---- | ------------- | ---- |
| 0              | 1               | 0           | 0.5  | 0.1  | 0             | 11   |
| 20             | 1.1             | 0           | 0.5  | 0.1  | 0             | 11.5 |
| 40             | 1.2             | 0           | 0.5  | 0.1  | 0             | 12   |
