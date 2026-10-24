# README

## ファイル構成

+または*で挟まれたものは github 上には上がっていない

- +で挟まれたディレクトリはダウンロードの必要あり
- *で挟まれたディレクトリ下のファイルはプログラムで自動生成される

```
.
├── main.cpp
├── enumerate_view.cpp
├── dataset
│   ├── +dbpedia+
│   │   ├── dbpedia_entity.json
│   │   ├── dbpedia_label.json
│   │   └── dbpedia_link.json
│   ├── nell
│   │   ├── nell_entity.json
│   │   ├── nell_label.json
│   │   └── nell_link.json
│   └── testpedia
│       ├── testpedia_entity.json
│       ├── testpedia_label.json
│       ├── testpedia_link.json
│       └── testpedia.pptx
├── query
│   ├── dbpedia
│   │   ├── ath-jpn_ath
│   │   │   ├── context.json
│   │   │   └── target.json
│   │   ├── ath-jpn_pers
│   │   │   ├── context.json
│   │   │   └── target.json
│   │   ├── game-prdc
│   │   │   ├── context.json
│   │   │   └── target.json
│   │   ├── game-prdc-all
│   │   │   ├── context.json
│   │   │   └── target.json
│   │   ├── jpngame
│   │   │   ├── context.json
│   │   │   └── target.json
│   │   ├── jpop-mscartist
│   │   │   ├── context.json
│   │   │   └── target.json
│   │   └── queries.json
│   ├── nell
│   │   ├── ath-all-all
│   │   │   ├── context.json
│   │   │   └── target.json
│   │   ├── mscartist-all-all
│   │   │   ├── context.json
│   │   │   └── target.json
│   │   ├── scitist-all
│   │   │   ├── context.json
│   │   │   └── target.json
│   │   └── queries.json
│   └── testpedia
│       ├──pers-all
│       │   ├── context.json
│       │   └── target.json
│       ├──pers-all-all
│       │   ├── context.json
│       │   └── target.json
│       └── queries.json
├── *enumerate*
│   ├── dbpedia
│   │   ├── CombinationEnumerate.json
│   │   ├── EntityEnumerate.json
│   │   └── LinkEnumerate.json
│   ├── nell
│   │   ├── CombinationEnumerate.json
│   │   ├── EntityEnumerate.json
│   │   └── LinkEnumerate.json
│   └── testpedia
│       ├── CombinationEnumerate.json
│       ├── EntityEnumerate.json
│       └── LinkEnumerate.json
├── *index*
│   ├── dbpedia
│   │   └── index.json
│   ├── nell
│   │   └── index.json
│   └── testpedia
│       └── index.json
├── config
│   └── outlier.json
└── +rapidjson+
    └── ...
```

## 環境構築

- rapidjson: [ここ](https://github.com/Tencent/rapidjson/ "GitHub")からダウンロード
- dataset/dbpedia: [ここ](https://drive.google.com/file/d/1GCykGt5Pa3fBjXeHA7qDT8ND35NldNdn/view?usp=share_link "Google Drive")からダウンロード（データサイズが大きいため Google Drive にアップロード）
- enumerate, index 中のファイル: main.cpp で生成

## main.cpp

- 例外検出を行う
- `g++ -o main main.cpp` でコンパイル
- まず，`./main -d <データセット> -f 1` でインデックスや一覧ファイルを生成
- 次に，`./main -d <データセット> -q <クエリ> -k <出力トリプル数> -s <スコアの種類> -o <探索方法> -f 0` で例外検出
- 実行例
  - `./main -d dbpedia -f 1`
  - `./main -d dbpedia -q queries -k 100 -s dist -o 0 -f 0`

**各パラメータ**

d

- データセット名
- "dbpedia"か"nell"か"testpedia"を指定

q

- クエリファイル名
- query/<データセット名> 中にある，クエリディレクトリ名を列挙したJSONファイルを読み込み

k

- 表示するトリプル数
- 省略すると100で実行

s

- スコアの種類
- "dist"で分布の偏りベース，"freq"で頻度ベースのスコアを使用
- 省略またはそれ以外の入力の場合は"dist"で実行

o

- 探索方法
- 0で枝刈りを用いた提案手法，1で全探索手法，2でコンテキストにマッチするパターンの集計のみを行う
- 省略またはそれ以外の入力の場合は0で実行

f

- 最初の実行かどうか
- 0で例外検出の実行，1でインデックスや一覧ファイルの生成
- 省略すると1で実行

## クエリの書き方

- context.json にコンテキスト情報， target.json にターゲット情報を入力し，それらを1つのディレクトリにまとめる
- このディレクトリ名をqで指定したクエリファイル名にJSON形式で列挙
- ワイルドカードの指定の際には， f=0 での実行時に `RG wildcard ≧ n` が表示されるので，このn以上の数値を入れることで指定可能

**コンテキスト**

```
[
    {
        "Entity":[
            {"EntityID":<エンティティラベルまたはエンティティID>, "EntityKind":<エンティティラベルならtrue，エンティティならfalse>},
            ...,
            {<他のエンティティ情報>}
        ],
        "Link":[
            {"HeadID":<headが何番目のエンティティか>, "LinkID":<linkのID>, "TailID":<tailが何番目のエンティティか>},
            ...,
            {<他のトリプル情報}}
        ]
    },
    ...,
    {<他のコンテキスト情報>}
]
```

**ターゲット**

```
[
    {
        "HeadID":<headのエンティティラベルまたはエンティティID>,
        "HeadKind":<headがエンティティラベルかどうか>,
        "LinkID":<linkのID>,
        "TailID":<tailのエンティティラベルまたはエンティティID>,
        "TailKind":<tailがエンティティラベルかどうか>
    },
    ...,
    {<他のターゲット情報>}
]
```

## enumerate_view.cc

- enumerate中のファイルを読み込み，エンティティ，エンティティラベル，リンクのID等を表示し，コンテキスト・ターゲット入力の手助けをする
- `g++ -o ev enumerate_view.cpp` でコンパイル
- 事前に `./main -d <データセット> -f 1` でインデックスや一覧ファイルを生成
- その後，`./enumerate_view -d <データセット>` で例外検出
- 実行例
  - `./enumerate_view -d dbpedia`

**各パラメータ**

d

- データセット名
- "dbpedia"か"nell"か"testpedia"を指定

**コマンド入力**

- `e <ID>` で，IDが<ID>のエンティティの名前とそのエンティティラベルを出力
- `e <エンティティ名>` で，<エンティティ名>を名前に含むエンティティのIDとそのエンティティラベルを出力
- `el <ID>` で，IDが<ID>のエンティティラベルの名前とそれにマッチするエンティティを出力
- `el 0` で，全てのエンティティラベルIDとエンティティラベル名を出力
- `el <ラベル名>` で，<ラベル名>を名前に含むエンティティラベルのIDを出力
- `r <ID>` で，IDが<ID>のリンクの名前を出力
- `r <リンク名>` で，<リンク名>を名前に含むリンクのIDを出力
- `c <ID>` で，IDが<ID>のエンティティラベルの名前とそれにマッチするエンティティの隣接情報を出力

**利用例**
<table>
  <thead>
    <tr>
      <th>状況</th> <th>コマンド</th>
    </tr>
  </thead>
  <tr>
    <td> エンティティID=3のエンティティ情報を調べたい </td> <td>e 3</td>
  </tr>
  <tr>
    <td> japanを名前に含むエンティティのIDを調べたい </td> <td>e japan</td>
  </tr>
  <tr>
    <td> エンティティラベルID=3のエンティティ情報を調べたい </td> <td>el 3</td>
  </tr>
  <tr>
    <td> 全てのエンティティラベルのID対応を調べたい </td> <td>el 0</td>
  </tr>
  <tr>
    <td> musicを名前に含むエンティティラベルのIDを調べたい </td> <td>el music</td>
  </tr>
  <tr>
    <td> リンクID=3のエンティティ情報を調べたい </td> <td>r 3</td>
  </tr>
  <tr>
    <td> childを名前に含むリンクのIDを調べたい </td> <td>r child</td>
  </tr>
  <tr>
    <td> エンティティラベルID=3のエンティティがどのようなリンク・エンティティラベルと隣接してるか調べたい </td> <td>c 3</td>
  </tr>
</table>
